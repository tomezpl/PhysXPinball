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
		std::cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << std::endl;

		//check all pairs
		for (physx::PxU32 i = 0; i < nbPairs; i++)
		{
			//if (static_cast<Pinball::Middleware::UserData*>(pairs[i].shapes[0]->getActor()->userData)->isCollider && static_cast<Pinball::Middleware::UserData*>(pairs[i].shapes[1]->getActor()->userData)->isCollider)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					std::cerr << "onContact::eNOTIFY_TOUCH_FOUND" << std::endl;
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
				std::cerr << "onContact::eNOTIFY_TOUCH_LOST" << std::endl;
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

	// Both objects need to contain each other's IDs in their filtermasks in order for a contact callback to be triggered.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		// If not a trigger, then generate contact response
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	return physx::PxFilterFlag::eDEFAULT;
}

struct FilterGroup
{
	enum Enum
	{
		eBALL = (1 << 0),
		eFLIPPER = (1 << 1),
		eTABLE = (1 << 2),
	};
};

void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // the FilterGroup this object identifies with
	filterData.word1 = filterMask; // the FilterGroup this object needs to collide with

	const physx::PxU32 numShapes = actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape*[numShapes];

	actor->getShapes(shapes, numShapes);

	// Set this filter data for all shapes of this object
	for (int i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}

	delete[] shapes;
}

int main(int* argc, char** argv)
{
	// Create renderer
	Pinball::Renderer gfx("Pinball Game");

	// Shaders
	GLuint diffuseShader, unlitShader;
	diffuseShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Diffuse.vert"), getFileContents("GLSL/Diffuse.frag"));
	unlitShader = Pinball::Renderer::compileShader(getFileContents("GLSL/Unlit.vert"), getFileContents("GLSL/Unlit.frag"));

	bool running = true;

	// Vertices for a triangle
	std::vector<float> verts = {
		-0.5f, -0.5f, 1.f, // bottom-left
		0.5f, -0.5f, 1.f, // bottom-right
		0.0f, 0.5f, 1.f // top
	};

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

	Pinball::Level level("Models/level_meshes.obj", "Models/level_origins.obj", cooking);

	physx::PxVec3 hingeLocation = level.HingeL()->Transform().p + (level.FlipperR()->Transform().p - level.HingeL()->Transform().p) * 0.9;
	
	hingeLocation = level.FlipperL()->Transform().p;
	((physx::PxRigidDynamic*)level.FlipperL()->GetPxActor())->setMass(0.f);
	((physx::PxRigidDynamic*)level.FlipperR()->GetPxActor())->setMass(0.f);
	((physx::PxRigidDynamic*)level.FlipperL()->GetPxActor())->setMassSpaceInertiaTensor(physx::PxVec3(0.f, 10.f, 0.f));
	((physx::PxRigidDynamic*)level.FlipperR()->GetPxActor())->setMassSpaceInertiaTensor(physx::PxVec3(0.f, 10.f, 0.f));

	boxObj.Transform(physx::PxTransform(hingeLocation));

	//levelObjects["FlipperL"].Transform(physx::PxTransform(physx::PxIdentity));

	flipperJointL = physx::PxSphericalJointCreate(*pxPhysics,
		(physx::PxRigidActor*)level.HingeL()->GetPxActor(), physx::PxTransform(hingeLocation - level.HingeL()->Transform().p),
		(physx::PxRigidActor*)level.FlipperL()->GetPxActor(), physx::PxTransform(physx::PxVec3(0.0f)));


	//flipperJointL->setLimit(physx::PxJointAngularLimitPair(-physx::PxPi / 4, physx::PxPi / 4));
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);

	//flipperJointL->setDriveVelocity(1.0f);
	level.FlipperL()->GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
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
	hingeLocation = level.FlipperR()->Transform().p;

	flipperJointR = physx::PxSphericalJointCreate(*pxPhysics,
		(physx::PxRigidActor*)level.HingeR()->GetPxActor(), physx::PxTransform(hingeLocation - level.HingeR()->Transform().p),
		(physx::PxRigidActor*)level.FlipperR()->GetPxActor(), physx::PxTransform(physx::PxVec3(0.0f)));
	level.FlipperR()->GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	//((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setCMassLocalPose(physx::PxTransform(levelObjects["FlipperR"].Geometry().GetCenterPoint() * 2.0f));


	flipperJointR->setLimitCone(physx::PxJointLimitCone(physx::PxPi / 4, physx::PxPi / 4, 0.01f));
	flipperJointR->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);

	setupFiltering((physx::PxRigidActor*)level.FlipperL()->GetPxActor(), FilterGroup::eFLIPPER, FilterGroup::eBALL);
	setupFiltering((physx::PxRigidActor*)level.FlipperR()->GetPxActor(), FilterGroup::eFLIPPER, FilterGroup::eBALL);
	setupFiltering((physx::PxRigidActor*)level.HingeL()->GetPxActor(), FilterGroup::eTABLE, FilterGroup::eBALL);
	setupFiltering((physx::PxRigidActor*)level.HingeR()->GetPxActor(), FilterGroup::eTABLE, FilterGroup::eBALL);

	setupFiltering((physx::PxRigidActor*)level.Ball()->GetPxActor(), FilterGroup::eBALL, FilterGroup::eTABLE | FilterGroup::eFLIPPER);
	setupFiltering((physx::PxRigidActor*)level.Ramp()->GetPxActor(), FilterGroup::eTABLE, FilterGroup::eBALL);
	setupFiltering((physx::PxRigidActor*)level.Table()->GetPxActor(), FilterGroup::eTABLE, FilterGroup::eBALL);

	tableObj.Geometry().Color(0.375f, 0.375f, 0.375f);
	ballObj.Geometry().Color(0.5f, 0.5f, 1.f);

	//scene->addActor(*boxObj.GetPxActor());
	scene->addActor(*planeObj.GetPxActor());
	scene->addActors(level.AllActors(), level.NbActors());

	planeObj.Geometry().Color(1.0f, 1.0f, 1.0f);
	planeObj.Transform(physx::PxTransform(physx::PxVec3(0.0f, -3.0f, 0.0f), physx::PxQuat(physx::PxIdentity)));

	double deltaTime = 0.0;
	double elapsedTime = glfwGetTime();

	bool paused = false;

	// Initial camera location
	Pinball::Camera cam(physx::PxTransform(physx::PxVec3(0.0f, -3.3f, 25.0f), physx::PxQuat(glm::radians(60.0f), physx::PxVec3(1.f, 0.f, 0.f))));

	// Prepare lights
	std::vector<Pinball::Light> lights = {
		Pinball::Light(glm::vec3(), glm::vec3(10.0f, -5.0f, 7.5f), glm::vec3(1.0, 1.0, 1.0)), // Sun light
		Pinball::Light(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)),
		Pinball::Light(glm::vec3(12.5f, 10.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 0.0f)),
		Pinball::Light(glm::vec3(-12.5f, 10.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 0.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 10.0f, -10.0f), glm::vec3(), glm::vec3(0.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(12.5f, 10.0f, -10.0f), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f))
	};

	// Sun light is a directional light, therefore it needs the direction vector
	lights[0].dir = glm::vec3(10.f, -5.f, 7.5f);

	float launchStrength = 0.0f;
	bool buildUp = false;

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
			((physx::PxRigidDynamic*)level.FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f)* 50.f);
			//((physx::PxRigidDynamic*)boxObj.GetPxActor())->addForce(physx::PxVec3(0.f, 1.f, -2.f) * 200);
		}
		else
		{
			((physx::PxRigidDynamic*)level.FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, -1.f, 0.f) * 25.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)level.FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f) * -50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)level.FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, 1.f, 0.f) * 25.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		{
			launchStrength += 20.0f;
			buildUp = true;
		}
		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE && buildUp)
		{
			((physx::PxRigidDynamic*)level.Ball()->GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, -launchStrength));
			launchStrength = 0.0f;
			buildUp = false;
		}

		// Process logic, prepare scene
		double prevElapsedTime = elapsedTime;
		elapsedTime = glfwGetTime();
		deltaTime = elapsedTime - prevElapsedTime;

		// Simulate physics
		if (!paused)
		{
			scene->simulate(deltaTime);
			scene->fetchResults(true);
		}

		// Attach light to ball
		physx::PxVec3 bp = level.Ball()->Transform().p;
		lights[1].pointPos = glm::vec3(bp.x, bp.y + 10.0f, bp.z);

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//gfx.Draw(boxObj, cam, lights, &diffuseShader);
		for (size_t i = 0; i < level.NbActors(); i++)
		{
			gfx.Draw(*level.At(i), cam, lights, &diffuseShader);
		}
		//gfx.Draw(planeObj, cam, lights, &unlitShader);
		//drawMesh(planeObj, glm::vec2(vWidth, vHeight), vao, vbo, ibo, unlitShader);

		glfwSwapBuffers(gfx.Window());
	}

	glfwDestroyWindow(gfx.Window());

	scene->release();

	PxCloseExtensions();

	return 0;
}