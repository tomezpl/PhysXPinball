// Tomasz Zajac (ZAJ15618587), CGP3012M Assignment 1 (2020)
// Pinball Game
//
// # BUILDING:
// ## Dependencies:
// The game requires GLFW3 for OpenGL window creation, GLM for mathematics, 
// Assimp for importing level data from .obj files and PhysX for physics simulation.
// 
// GLFW3, GLM and Assimp are added to this project via NuGet. They should be resolved automatically.
// PhysX 3.4.2 is included via the PHYSX_SDK environment variable (Windows).
//
// # USAGE:
// The game sets the ball in the plunger area. Holding down Right Shift key will build the ball's launch force,
// releasing Right Shift key will launch the ball.
//
// Left and Right arrow keys are used to control the flippers.
//
// If the ball touches the bottom of the table and is too slow, the player loses.
//
// Flippers are implemented using spherical joints. In theory, revolute joints should have been used instead,
// but I could not get angular motion to work properly on the Y-axis using revolute. 
// The flippers have infinite mass and their angular velocity is set on a per-frame basis, so they should behave just like revolute joints would.
//
// CCD is enabled for the flippers & ball to avoid tunneling at high velocities or low framerate/instable simulation steps.
// The simulation callback is able to cause effects in the game by the global game state struct.
//
// Different objects in the game have different physic materials, properties and behaviours.
// For example, the ball and the bumpers have very high restitution, making them bouncy.
// The table has some slight friction to avoid the ball constantly bouncing up and down when it hits the bottom of the table.
// When the ball slides on the ramp at the top of the table, it will gain a "boost", multiplying its current XZ velocity by a scalar.
//
// When a ball scratches a surface, it emits spark particles at the contact point.
// 
// # REFERENCES, EXTERNAL SOURCES:
// NVIDIA's PhysX manual, Web forums and the SDK sample code (e.g. RendererCapsuleShape) was reused in parts of this program.
// Sean Barrett's (github:nothings) stb_image.h was used for drawing 2D images, such as the "game over" screen.

// C++ std libs
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <iostream>

// Game engine & PhysX
#include "GameObject.h"
#include "Level.h"
#include "Light.h"
#include "Renderer.h"
#include "Util.h"

// Level containing all GameObjects and particles
Pinball::Level* gLevel = nullptr;

// Game state. Kept as a global struct so it can be accessed from the simulation callback.
struct
{
	// Should the game trigger the game-over state?
	bool notifyLoss = false;

	// For boosting the ball when sliding across the ramp
	bool rampBoostActive = false;

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

///A customised collision class, implementing various callbacks
class MySimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
	MySimulationEventCallback() {}

	///Method called when the contact with the trigger object is detected.
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) { }

	///Method called when the contact by the filter shader is detected.
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		// Check for collision with particular objects
		// TODO: this *could* be faster (and safer) by checking simulation filter flags instead
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
				// Trigger game-over state if the ball is at the bottom of the table (not in the plunger area) and below minimal velocity.
				if (ballPos.z >= gGameState.gameOverArea.z && std::fabs(ballPos.x - gGameState.plungerArea.x) > 0.5f && ballVelocity.magnitude() <= gGameState.gameOverVelocity)
				{
					gGameState.notifyLoss = true;
				}
			}
		}

		// Check all pairs for contact points
		for (unsigned int i = 0; i < nbPairs; i++)
		{
			const physx::PxContactPair& cp = pairs[i];

			physx::PxContactStreamIterator iter(cp.contactPatches, cp.contactPoints, cp.getInternalFaceIndices(), cp.patchCount, cp.contactCount);

			unsigned int nbContacts = 0;

			while (iter.hasNextPatch())
			{
				iter.nextPatch();
				while (iter.hasNextContact())
				{
					iter.nextContact();
					physx::PxVec3 point = iter.getContactPoint();

					// Emit spark particles from the contact point
					if (ballFound & !floorFound)
					{
						gGameState.newParticleOrigin = point;
					}

					nbContacts++;
				}
			}

			// Check for persistent contact
			if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				if (rampFound && ballFound)
				{
					gGameState.rampBoostActive = true;
				}
			}

			// Check for lost contact
			if (pairs[i].events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
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

	// Game-over screen image
	Pinball::Image gameOverImg("Images/gameover.png");
	gfx.CreateTexture(gameOverImg);

	// PhysX
	physx::PxDefaultAllocator pxAlloc;
	physx::PxDefaultErrorCallback pxErrClb;
	physx::PxPvd* pxPvd = nullptr;

	// Create PxFoundation. This is used for all resources in PhysX
	physx::PxFoundation* pxFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, pxAlloc, pxErrClb);

	// Link with PhysX Visual Debugger. This might cause crashes, in which case delete this.
	pxPvd = physx::PxCreatePvd(*pxFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	pxPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	// Create the PhysX physics API
	physx::PxPhysics* pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), false, pxPvd);

	// Initialise extensions - PVD can crash without this.
	PxInitExtensions(*pxPhysics, pxPvd);

	// Create cooking. This is used for creating geometry/shapes.
	physx::PxCookingParams cookingParams = physx::PxCookingParams(physx::PxTolerancesScale());
	physx::PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION, PxGetPhysics().getFoundation(), cookingParams);

	// Scene descriptor
	physx::PxSceneDesc sceneDesc = physx::PxSceneDesc(physx::PxTolerancesScale());
	// Set gravity to push downwards on the Y-axis and inwards on the Z-axis, so that the ball falls down the table.
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 9.81f);
	sceneDesc.filterShader = MyFilterShader;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	// Enable continous collision detection - this will be used for the ball & flippers
	sceneDesc.flags = physx::PxSceneFlag::eENABLE_CCD;

	// Create scene and point it to the simulation callback
	physx::PxScene* scene = PxGetPhysics().createScene(sceneDesc);
	scene->setSimulationEventCallback(new MySimulationEventCallback());

	// Load the level from the Blender-exported file and point it to our PhysX scene.
	gLevel = new Pinball::Level("Models/level_meshes.obj", "Models/level_origins.obj", cooking);
	gLevel->SetScene(scene);

	scene->addActors(gLevel->AllActors(), gLevel->NbActors());

	// Time from last frame
	double deltaTime = 0.0;
	// Total time
	double elapsedTime = glfwGetTime();

	bool paused = false;

	// Initial camera location
	Pinball::Camera cam(physx::PxTransform(physx::PxVec3(0.0f, -3.3f, 25.0f), physx::PxQuat(glm::radians(60.0f), physx::PxVec3(1.f, 0.f, 0.f))));

	// Prepare lights
	std::vector<Pinball::Light> lights = {
		Pinball::Light(glm::vec3(), glm::vec3(10.f, -5.f, 7.5f), glm::vec3(1.0, 1.0, 1.0)), // Sun light
		//
		// Point lights:
		Pinball::Light(glm::vec3(12.5f, 2.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 2.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 2.0f, -10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(12.5f, 2.0f, -10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(0.5f, 2.0f, 0.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f))
	};

	// Initial launch force for the ball plunger
	float launchStrength = 0.0f;
	// Is the player still adding force? (holding down Right Shift)
	bool buildUp = false;

	// Store plunger area location (taken from the ball's initial position)
	gGameState.plungerArea = gLevel->Ball()->Transform().p;
	// Set the game over area to be at the same Z-coordinate as the plunger area, since they are both at th bottom of the table
	gGameState.gameOverArea = physx::PxVec3(0.f, 0.f, gGameState.plungerArea.z);

	// Is the game running? Set to false when window is closed
	bool running = true;

	while (running)
	{
		// This tracks if the spacebar key was tapped this frame.
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

		// If spacebar was tapped, toggle the pause mode
		if (glfwGetKey(gfx.Window(), GLFW_KEY_SPACE) == GLFW_RELEASE && spacePressed)
		{
			paused = !paused;
		}

		// Point the left flipper upwards if left arrow is pressed
		if (glfwGetKey(gfx.Window(), GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f) * 50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)gLevel->FlipperL()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, -1.f, 0.f) * 25.f);
		}

		// Point the right flipper upwards if right arrow is pressed
		if (glfwGetKey(gfx.Window(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.f, 1.f, 0.f) * -50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)gLevel->FlipperR()->GetPxActor())->setAngularVelocity(physx::PxVec3(0.0f, 1.f, 0.f) * 25.f);
		}

		// Keep "building up" an initial force for the ball plunger while Shift key is pressed
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

		// Update particle state, delete any dead particles
		gLevel->UpdateParticles(deltaTime);

		// Simulate physics
		if (!paused)
		{
			scene->simulate(deltaTime);
			scene->fetchResults(true);
		}

		// Check if game-over state was triggered (ie. after ball hit the bottom of the table)
		// There is a small delay before respawning to display the "Game Over" screen
		if (gGameState.notifyLoss)
		{
			gGameState.gameOverTime += deltaTime;
		}

		// Respawn if the "game over" screen duration has passed
		if (gGameState.gameOverDuration < gGameState.gameOverTime)
		{
			gLevel->Ball()->Transform(physx::PxTransform(gGameState.plungerArea));
			gGameState.notifyLoss = false;
			gGameState.gameOverTime = 0.0f;
		}

		// Ball velocity
		physx::PxVec3 ballV = ((physx::PxRigidDynamic*)gLevel->Ball()->GetPxActor())->getLinearVelocity();

		// Spawn particles around ball upon contact
		if (gGameState.spawnParticles)
		{
			// minimum velocity to spawn spark particles
			float minSpeed = 1.0f;
			if (ballV.magnitude() > minSpeed)
			{
				gLevel->SpawnParticles(cooking, 3, Pinball::ParticleType::ePARTICLE_SPARK, gGameState.newParticleOrigin);
			}
		}

		// Increase the ball's velocity if it's sliding on the ramp
		if (gGameState.rampBoostActive)
		{
			const float boost = 1.025f; // XZ boost given to the ball's velocity when sliding across the ramp
			((physx::PxRigidDynamic*)gLevel->Ball()->GetPxActor())->setLinearVelocity(physx::PxVec3(ballV.x*boost, ballV.y, ballV.z*boost));
			gGameState.rampBoostActive = false;
		}

		// Render the level

		// Clear last frame
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw each actor
		for (size_t i = 0; i < gLevel->NbActors(); i++)
		{
			gfx.Draw(*gLevel->At(i), cam, lights, &diffuseShader);
		}

		// Draw spark particles
		gfx.DrawParticles(*gLevel, cam, &sparkShader);

		// Draw game-over screen if necessary
		if (gGameState.notifyLoss)
		{
			gfx.DrawImage(gameOverImg, 0.f, 0.f, 1.f, 1.f, &imgShader);
		}

		// Send to front buffer
		glfwSwapBuffers(gfx.Window());
	}

	// Cleanup
	glfwDestroyWindow(gfx.Window());
	scene->release();
	PxCloseExtensions();
	delete gLevel;

	return 0;
}