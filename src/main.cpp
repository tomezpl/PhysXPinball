// C++ std libs
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

// Game engine & PhysX
#include "GameObject.h"
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
			if (static_cast<Pinball::Middleware::UserData*>(pairs[i].shapes[0]->getActor()->userData)->isCollider && static_cast<Pinball::Middleware::UserData*>(pairs[i].shapes[1]->getActor()->userData)->isCollider)
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
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
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

	// Create game objects
	for (size_t i = 0; i < levelMeshes.size(); i++)
	{
		Pinball::GameObject::Type objectType = Pinball::GameObject::Static;
		Pinball::GameObject* objToAssignMesh = nullptr;
		if (strContains("BallFlipperLFlipperR", levelMeshes[i].Name()))
		{
			objectType = Pinball::GameObject::Dynamic;
			//objToAssignMesh = &ballObj;
		}

		if (strContains(levelMeshes[i].Name(), "Table"))
		{
			//objToAssignMesh = &tableObj;
		}

		if (objToAssignMesh == nullptr)
		{
			levelObjects[levelMeshes[i].Name()] = (Pinball::GameObject());
			objToAssignMesh = &levelObjects[levelMeshes[i].Name()];
		}

		if (objToAssignMesh != nullptr)
		{
			objToAssignMesh->Geometry(levelMeshes[i], objectType);
			objToAssignMesh->Transform(physx::PxTransform(levelOrigins[i].GetCenterPoint(), physx::PxQuat(physx::PxIdentity)));
			scene->addActor(*objToAssignMesh->GetPxActor());
			objToAssignMesh->Geometry().Color(0.5f, 0.5f, 0.5f);
			/*if (strContains(levelMeshes[i].Name(), "Table"))
			{
				objToAssignMesh->Transform(physx::PxTransform(physx::PxVec3(0.0f, -3.0f, 0.0f)));
			}*/
		}
	}

	((physx::PxRigidActor*)levelObjects["FlipperL"].GetPxActor());

	flipperJointL = physx::PxSphericalJointCreate(*pxPhysics,
		(physx::PxRigidActor*)levelObjects["HingeL"].GetPxActor(), physx::PxTransform((levelObjects["FlipperL"].Transform().p - levelObjects["HingeL"].Transform().p)),
		(physx::PxRigidActor*)levelObjects["FlipperL"].GetPxActor(), physx::PxTransform(physx::PxIdentity) /*physx::PxTransform(levelObjects["FlipperL"].Geometry().GetCenterPoint() * -2.0f)*/);

	//flipperJointL->setDriveVelocity(1.0f);
	levelObjects["FlipperL"].GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	boxObj.GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	((physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor())->setCMassLocalPose(physx::PxTransform(levelObjects["FlipperL"].Geometry().GetCenterPoint() * 2.0f));
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	flipperJointL->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, true);
	//flipperJointL->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_FREESPIN, true);

	boxObj.Transform(physx::PxTransform(levelObjects["FlipperL"].Transform().p + levelObjects["FlipperL"].Geometry().GetCenterPoint() * 3.f, physx::PxQuat(physx::PxIdentity)));

	/*physx::PxRevoluteJoint* flipperJointLPointer = physx::PxRevoluteJointCreate(*pxPhysics,
		(physx::PxRigidActor*)levelObjects["FlipperL"].GetPxActor(), physx::PxTransform(levelObjects["FlipperL"].Geometry().GetCenterPoint() * 3.0f),
		((physx::PxRigidActor*)(boxObj.GetPxActor())), physx::PxTransform(physx::PxIdentity)
	);*/

	flipperJointR = physx::PxSphericalJointCreate(*pxPhysics,
		(physx::PxRigidActor*)levelObjects["HingeR"].GetPxActor(), physx::PxTransform(-1.0f * (levelObjects["HingeR"].Transform().p - levelObjects["FlipperR"].Transform().p)),
		(physx::PxRigidActor*)levelObjects["FlipperR"].GetPxActor(), physx::PxTransform(physx::PxIdentity));
	levelObjects["FlipperR"].GetPxActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setCMassLocalPose(physx::PxTransform(levelObjects["FlipperR"].Geometry().GetCenterPoint() * 2.0f));
		
	tableObj.Geometry().Color(0.375f, 0.375f, 0.375f);
	ballObj.Geometry().Color(0.5f, 0.5f, 1.f);

	//scene->addActor(*boxObj.GetPxActor());
	scene->addActor(*planeObj.GetPxActor());

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

		if (glfwGetKey(gfx.Window(), GLFW_KEY_UP) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, -20.0f));
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			//((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(-20.f, 0.0f, 0.0f));
			//physx::PxRigidBodyExt::addForceAtLocalPos(*(physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor(), physx::PxVec3(-1.f, -1.f, 2.f) * 100.f, levelObjects["FlipperL"].Geometry().GetCenterPoint() * 1.f);
			((physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor())->setLinearVelocity(physx::PxVec3(1.5f, 0.f, -1.f)*50.f);
			//((physx::PxRigidDynamic*)boxObj.GetPxActor())->addForce(physx::PxVec3(0.f, 1.f, -2.f) * 200);
		}
		else
		{
			((physx::PxRigidDynamic*)levelObjects["FlipperL"].GetPxActor())->setLinearVelocity(physx::PxVec3(-1.5f, 0.f, -1.f) * -50.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, 10.0f));
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setLinearVelocity(physx::PxVec3(-1.5f, 0.f, -1.f) * 50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setLinearVelocity(physx::PxVec3(1.5f, 0.f, -1.f) * -50.f);
		}

		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		{
			launchStrength += 20.0f;
			buildUp = true;
		}
		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE && buildUp)
		{
			((physx::PxRigidDynamic*)levelObjects["Ball"].GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, -launchStrength));
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
		physx::PxVec3 bp = levelObjects["Ball"].Transform().p;
		lights[1].pointPos = glm::vec3(bp.x, bp.y + 10.0f, bp.z);

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gfx.Draw(boxObj, cam, lights, &diffuseShader);
		for (std::map<std::string, Pinball::GameObject>::iterator it = levelObjects.begin(); it != levelObjects.end(); it++)
		{
			gfx.Draw(it->second, cam, lights, &diffuseShader);
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