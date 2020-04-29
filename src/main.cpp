// C++ std libs
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

// Game engine & PhysX
#include "GameObject.h"
#include "Level.h"
#include "Middleware.h"
#include "Light.h"
#include "Renderer.h"
#include "Util.h"

Pinball::Level* gLevel = nullptr;

// Game state. Kept as a global struct so it can be accessed from the simulation callback.
struct
{
	// Should the game trigger the game-over state?
	bool notifyLoss = false;

	// For spawning particles upon ball contact
	bool spawnParticles = false;
	physx::PxVec3 newParticleOrigin = physx::PxVec3();

	// Coordinates of the plunger area at spawn, to avoid counting that as a loss
	physx::PxVec3 plungerArea = physx::PxVec3();

	// Coordinates of the game-over area (namely the Z-coordinate would be used to determine game-over state)
	physx::PxVec3 gameOverArea = physx::PxVec3();

	// Minimum velocity allowed within the game-over area before game-over state is triggered.
	// In other words, if the ball is kept above this velocity, the player is still able to get it back to the play area.
	const float gameOverVelocity = 3.0f;

	// Game-Over screen duration
	const float gameOverDuration = 3.0f;
	// Game-Over screen time so far
	float gameOverTime = 0.0f;
} gGameState;

///A customised collision class, implemneting various callbacks
class MySimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
	//an example variable that will be checked in the main simulation loop
	bool trigger;

	MySimulationEventCallback() : trigger(false) {}

	///Method called when the contact with the trigger object is detected.
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		//you can read the trigger information here
		for (physx::PxU32 i = 0; i < count; i++)
		{
			//filter out contact with non-triggers
			if (static_cast<Pinball::Middleware::UserData*>(pairs[i].otherActor->userData)->isTrigger)
			{
				//check if eNOTIFY_TOUCH_FOUND trigger
				if (pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					std::cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << std::endl;
					trigger = true;
				}
				//check if eNOTIFY_TOUCH_LOST trigger
				if (pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					std::cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << std::endl;
					trigger = false;
				}
			}
		}
	}

	///Method called when the contact by the filter shader is detected.
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		//std::cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << std::endl;

		// Check for collision with particular objects
		bool ballFound = strContains(pairHeader.actors[0]->getName(), "Ball") || strContains(pairHeader.actors[1]->getName(), "Ball");
		bool tableFound = strContains(pairHeader.actors[0]->getName(), "Table") || strContains(pairHeader.actors[1]->getName(), "Table");
		bool floorFound = strContains(pairHeader.actors[0]->getName(), "Floor") || strContains(pairHeader.actors[1]->getName(), "Floor");
		bool rampFound = strContains(pairHeader.actors[0]->getName(), "Ramp") || strContains(pairHeader.actors[1]->getName(), "Ramp");
		physx::PxRigidDynamic* ball = (physx::PxRigidDynamic*)gLevel->Ball()->GetPxActor();
		physx::PxVec3 ballVelocity = ball->getLinearVelocity();
		physx::PxVec3 ballPos = ball->getGlobalPose().p;

		if (ballFound)
		{
			gGameState.newParticleOrigin = ballPos;
			gGameState.spawnParticles = !floorFound; // don't generate spark particles on persistent contact with floor, there's too many of them
			if (tableFound)
			{
				// Trigger game-over state as 
				if (ballPos.z >= gGameState.gameOverArea.z && std::fabs(ballPos.x - gGameState.gameOverArea.x) > 0.5f && ballVelocity.magnitude() <= gGameState.gameOverVelocity)
				{
					gGameState.notifyLoss = true;
				}
			}
		}

		//check all pairs
		for (unsigned int i = 0; i < nbPairs; i++)
		{
			const physx::PxContactPair& cp = pairs[i];

			physx::PxContactStreamIterator iter(cp.contactPatches, cp.contactPoints, cp.getInternalFaceIndices(), cp.patchCount, cp.contactCount);

			unsigned int flippedContacts = (cp.flags & physx::PxContactPairFlag::eINTERNAL_CONTACTS_ARE_FLIPPED);
			unsigned int hasImpulses = (cp.flags & physx::PxContactPairFlag::eINTERNAL_HAS_IMPULSES);
			unsigned int nbContacts = 0;

			while (iter.hasNextPatch())
			{
				iter.nextPatch();
				while (iter.hasNextContact())
				{
					iter.nextContact();
					physx::PxVec3 point = iter.getContactPoint();

					if (ballFound & !floorFound)
					{
						gGameState.newParticleOrigin = point;
					}

					nbContacts++;
				}
			}

			//check eNOTIFY_TOUCH_FOUND
			if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				std::cerr << "onContact::eNOTIFY_TOUCH_FOUND" << std::endl;
			}
			if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				if (rampFound && ballFound)
				{
					const float boost = 1.025f; // XZ boost given to the ball's velocity when sliding across the ramp
					ballVelocity.x *= boost;
					ballVelocity.z *= boost;

					ball->setLinearVelocity(ballVelocity);
				}
			}
			//check eNOTIFY_TOUCH_LOST
			if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				std::cerr << "onContact::eNOTIFY_TOUCH_LOST" << std::endl;
				if (ballFound)
				{
					gGameState.spawnParticles = false;
				}
			}

		}
	}

	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	virtual void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
#if PX_PHYSICS_VERSION >= 0x304000
	virtual void onAdvance(const physx::PxRigidBody * const* bodyBuffer, const physx::PxTransform * poseBuffer, const physx::PxU32 count) {}
#endif
};

physx::PxFilterFlags MyFilterShader(
	/* Object A: */ physx::PxFilterObjectAttributes attribs0, physx::PxFilterData filterData0,
	/* Object B: */ physx::PxFilterObjectAttributes attribs1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSz)
{
	// Prevent trigger collision
	if (physx::PxFilterObjectIsTrigger(attribs0) || physx::PxFilterObjectIsTrigger(attribs1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	// Suppress particles
	if ((filterData0.word0 & Pinball::FilterGroup::ePARTICLE) || (filterData1.word0 & Pinball::FilterGroup::ePARTICLE))
	{
		return physx::PxFilterFlag::eKILL;
	}

	// Both objects need to contain each other's IDs in their filtermasks in order for a contact callback to be triggered.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		// If not a trigger, then generate contact response
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
	}

	if ((filterData0.word0 & Pinball::FilterGroup::eBALL) || (filterData1.word0 & Pinball::FilterGroup::eBALL))
	{
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_CCD;
		pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	return physx::PxFilterFlag::eDEFAULT;
}

int main(int* argc, char** argv)
{
	// Create renderer
	Pinball::Renderer gfx("Pinball Game");

	// Shaders
	GLuint diffuseShader, unlitShader, sparkShader, imgShader;
	diffuseShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Diffuse.vert"), getFileContents("GLSL/Diffuse.frag"));
	unlitShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Unlit.vert"), getFileContents("GLSL/Unlit.frag"));
	sparkShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Spark.vert"), getFileContents("GLSL/Spark.frag"));
	imgShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Image2D.vert"), getFileContents("GLSL/Image2D.frag"));

	// game-over screen image
	Pinball::Image gameOverImg("Images/gameover.png");
	gfx.CreateTexture(gameOverImg);

	bool running = true;

	// PhysX
	physx::PxDefaultAllocator pxAlloc;
	physx::PxDefaultErrorCallback pxErrClb;
	physx::PxPvd* pxPvd = nullptr;

	physx::PxFoundation* pxFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, pxAlloc, pxErrClb);
	pxPvd = physx::PxCreatePvd(*pxFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	pxPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	physx::PxPhysics* pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), false, pxPvd);
	PxInitExtensions(*pxPhysics, pxPvd);
	physx::PxCookingParams cookingParams = physx::PxCookingParams(physx::PxTolerancesScale());
	//cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	physx::PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION, PxGetPhysics().getFoundation(), cookingParams);

	physx::PxSceneDesc sceneDesc = physx::PxSceneDesc(physx::PxTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 9.81f);
	sceneDesc.filterShader = MyFilterShader;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.flags = physx::PxSceneFlag::eENABLE_CCD;
	physx::PxScene* scene = PxGetPhysics().createScene(sceneDesc);
	scene->setSimulationEventCallback(new MySimulationEventCallback());

	Pinball::Mesh boxMesh = Pinball::Mesh::createBox(cooking);
	boxMesh.Color(0.0f, 1.0f, 0.0f);
	Pinball::GameObject boxObj(boxMesh);
	boxObj.Transform(physx::PxTransform(physx::PxVec3(0.0f, 3.0f, 0.f), physx::PxQuat(physx::PxIdentity)));

	Pinball::Mesh planeMesh = Pinball::Mesh::createPlane(cooking);
	Pinball::GameObject planeObj(planeMesh, Pinball::GameObject::Type::Static);

	// TODO: change to std::map?
	std::vector<Pinball::Mesh> levelMeshes = Pinball::Mesh::fromFile("Models/level_meshes.obj", cooking);
	std::vector<Pinball::Mesh> levelOrigins = Pinball::Mesh::fromFile("Models/level_origins.obj", cooking, false);
	Pinball::GameObject tableObj;
	Pinball::GameObject ballObj;
	std::map<std::string, Pinball::GameObject> levelObjects;

	physx::PxSphericalJoint* flipperJointL = nullptr, *flipperJointR = nullptr;

	gLevel = new Pinball::Level("Models/level_meshes.obj", "Models/level_origins.obj", cooking);
	gLevel->SetScene(scene);

	physx::PxVec3 hingeLocation = gLevel->HingeL()->Transform().p + (gLevel->FlipperR()->Transform().p - gLevel->HingeL()->Transform().p) * 0.9;
	
	hingeLocation = gLevel->FlipperL()->Transform().p;
	((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setMass(0.f);
	((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setMass(0.f);
	((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setMassSpaceInertiaTensor(physx::PxVec3(0.f, 10.f, 0.f));
	((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setMassSpaceInertiaTensor(physx::PxVec3(0.f, 10.f, 0.f));

	boxObj.Transform(physx::PxTransform(hingeLocation));

	//levelObjects["FlipperL"].Transform(physx::PxTransform(physx::PxIdentity));

	flipperJointL = physx::PxSphericalJointCreate(*pxPhysics,
		gLevel->HingeL()->GetPxRigidActor(), physx::PxTransform(hingeLocation - gLevel->HingeL()->Transform().p),
		gLevel->FlipperL()->GetPxRigidActor(), physx::PxTransform(physx::PxVec3(0.0f)));


	//flipperJointL->setLimit(physx::PxJointAngularLimitPair(-physx::PxPi / 4, physx::PxPi / 4));
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);

	//flipperJointL->setDriveVelocity(1.0f);
	gLevel->FlipperL()->GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	boxObj.GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	//((physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor())->setCMassLocalPose(physx::PxTransform(levelObjects["FlipperL"].Geometry().GetCenterPoint() * 2.0f));
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f); 
	flipperJointL->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, true);
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_FREESPIN, true);

	flipperJointL->setLimitCone(physx::PxJointLimitCone(physx::PxPi / 4, physx::PxPi / 4, 0.01f));
	flipperJointL->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);

	//boxObj.Transform(physx::PxTransform(levelObjects["FlipperL"].Transform().p + levelObjects["FlipperL"].Geometry().GetCenterPoint() * 3.f, physx::PxQuat(physx::PxIdentity)));

	/*physx::PxRevoluteJoint* flipperJointLPointer = physx::PxRevoluteJointCreate(*pxPhysics,
		(physx::PxRigidActor*)levelObjects["FlipperL"].GetPxActor(), physx::PxTransform(levelObjects["FlipperL"].Geometry().GetCenterPoint() * 3.0f),
		((physx::PxRigidActor*)(boxObj.GetPxActor())), physx::PxTransform(physx::PxIdentity)
	);*/
	hingeLocation = gLevel->FlipperR()->Transform().p;

	flipperJointR = physx::PxSphericalJointCreate(*pxPhysics,
		gLevel->HingeR()->GetPxRigidActor(), physx::PxTransform(hingeLocation - gLevel->HingeR()->Transform().p),
		gLevel->FlipperR()->GetPxRigidActor(), physx::PxTransform(physx::PxVec3(0.0f)));
	gLevel->FlipperR()->GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	//((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setCMassLocalPose(physx::PxTransform(levelObjects["FlipperR"].Geometry().GetCenterPoint() * 2.0f));


	flipperJointR->setLimitCone(physx::PxJointLimitCone(physx::PxPi / 4, physx::PxPi / 4, 0.01f));
	flipperJointR->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);

	tableObj.Geometry().Color(0.375f, 0.375f, 0.375f);
	ballObj.Geometry().Color(0.5f, 0.5f, 1.f);

	//scene->addActor(*boxObj.GetPxActor());
	scene->addActor(*planeObj.GetPxActor());
	scene->addActors(gLevel->AllActors(), gLevel->NbActors());

	//Pinball::Particle testParticle(cooking, physx::PxVec3(0.f), Pinball::ParticleType::ePARTICLE_SPARK);

	//scene->addActor(*testParticle.GetPxActor());

	planeObj.Geometry().Color(1.0f, 1.0f, 1.0f);
	planeObj.Transform(physx::PxTransform(physx::PxVec3(0.0f, -3.0f, 0.0f), physx::PxQuat(physx::PxIdentity)));

	double deltaTime = 0.0;
	double elapsedTime = glfwGetTime();

	bool paused = false;

	// Initial camera location
	Pinball::Camera cam(physx::PxTransform(physx::PxVec3(0.0f, -3.3f, 25.0f), physx::PxQuat(glm::radians(60.0f), physx::PxVec3(1.f, 0.f, 0.f))));

	// Prepare lights
	std::vector<Pinball::Light> lights = {
		Pinball::Light(glm::vec3(), glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(1.0, 1.0, 1.0)), // Sun light
		Pinball::Light(glm::vec3(12.5f, 2.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 2.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 2.0f, -10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(12.5f, 2.0f, -10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(0.5f, 2.0f, 0.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f))
	};

	// Sun light is a directional light, therefore it needs the direction vector
	lights[0].dir = glm::vec3(10.f, -5.f, 7.5f);

	float launchStrength = 0.0f;
	bool buildUp = false;

	// Store plunger area location (taken from the ball's initial position)
	gGameState.plungerArea = gLevel->Ball()->Transform().p;
	gGameState.gameOverArea = gGameState.plungerArea;

	while (running)
	{
		bool spacePressed = false;
		if (glfwGetKey(gfx.Window(), GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			spacePressed = true;
		}

		// Process events
		glfwPollEvents();
		if (glfwWindowShouldClose(gfx.Window()))
		{
			running = false;
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_SPACE) == GLFW_RELEASE && spacePressed)
		{
			paused = !paused;
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			//((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(-20.f, 0.0f, 0.0f));
			//physx::PxRigidBodyExt::addForceAtLocalPos(*(physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor(), physx::PxVec3(-1.f, -1.f, 2.f) * 100.f, levelObjects["FlipperL"].Geometry().GetCenterPoint() * 1.f);
			((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f)* 50.f);
			//((physx::PxRigidDynamic*)boxObj.GetPxActor())->addForce(physx::PxVec3(0.f, 1.f, -2.f) * 200);
		}
		else
		{
			((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, -1.f, 0.f) * 25.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f) * -50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, 1.f, 0.f) * 25.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		{
			launchStrength += 40.0f;
			buildUp = true;
		}
		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE && buildUp)
		{
			((physx::PxRigidDynamic*)gLevel->Ball()->GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, -launchStrength));
			launchStrength = 0.0f;
			buildUp = false;
		}

		// Process logic, prepare scene
		double prevElapsedTime = elapsedTime;
		elapsedTime = glfwGetTime();
		deltaTime = elapsedTime - prevElapsedTime;

		gLevel->UpdateParticles(deltaTime);

		// Simulate physics
		if (!paused)
		{
			scene->simulate(deltaTime);
			scene->fetchResults(true);
		}

		// Check if ball hit bottom of table
		if (gGameState.notifyLoss)
		{
			gGameState.gameOverTime += deltaTime;
		}

		if (gGameState.gameOverDuration < gGameState.gameOverTime)
		{
			gLevel->Ball()->Transform(physx::PxTransform(gGameState.plungerArea));
			gGameState.notifyLoss = false;
			gGameState.gameOverTime = 0.0f;
		}

		// Spawn particles around ball upon contact
		if (gGameState.spawnParticles)
		{
			// minimum velocity to spawn spark particles
			float minSpeed = 3.0f;
			physx::PxVec3 ballV = ((physx::PxRigidDynamic*)gLevel->Ball()->GetPxActor())->getLinearVelocity();
			if (ballV.magnitude() > minSpeed)
			{
				gLevel->SpawnParticles(cooking, 3, Pinball::ParticleType::ePARTICLE_SPARK, gGameState.newParticleOrigin);
			}
		}

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//gfx.Draw(boxObj, cam, lights, &diffuseShader);
		for (size_t i = 0; i < gLevel->NbActors(); i++)
		{
			gfx.Draw(*gLevel->At(i), cam, lights, &diffuseShader);
		}

		gfx.DrawParticles(*gLevel, cam, &sparkShader);
		if (gGameState.notifyLoss)
		{
			gfx.DrawImage(gameOverImg, 0.f, 0.f, 1.f, 1.f, &imgShader);
		}
		//gfx.DrawParticle(*gLevel->Ball(), cam, &sparkShader);
		//gfx.Draw(planeObj, cam, lights, &unlitShader);
		//drawMesh(planeObj, glm::vec2(vWidth, vHeight), vao, vbo, ibo, unlitShader);

		glfwSwapBuffers(gfx.Window());
	}

	glfwDestroyWindow(gfx.Window());

	scene->release();

	PxCloseExtensions();

	delete gLevel;

	return 0;
}