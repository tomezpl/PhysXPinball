// C++ std libs
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <iostream>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>

// GL math
#include <glm/glm.hpp>
#include <glm/packing.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define TINYOBJLOADER_IMPLEMENTATION

// Game engine & PhysX
#include "GameObject.h"
#include "Middleware.h"
#include "Light.h"
#include "Util.h"

// Utility: returns contents of a file as text. Used to get shader sources.
std::string getFileContents(std::string path)
{
	std::ifstream fs(path);
	std::string ret = "";
	while (fs.good())
	{
		std::string currentLine = "";
		std::getline(fs, currentLine);
		ret += currentLine;
		if (!fs.eof())
			ret += "\n";
	}

	return ret;
}

glm::mat4* getTransform(Pinball::GameObject& obj, glm::vec3 camPos, glm::vec3 camRot, glm::vec2 viewport)
{
	glm::mat4* ret = new glm::mat4[3];

	physx::PxTransform worldTransform = obj.Transform();
	glm::vec3 modelPos = glm::vec3(worldTransform.p.x, worldTransform.p.y, worldTransform.p.z);
	glm::quat modelRot(worldTransform.q.w, worldTransform.q.x, worldTransform.q.y, worldTransform.q.z);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPos);
	model *= glm::mat4_cast(modelRot);
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // TODO: no scaling for now

	glm::mat4 view = glm::translate(glm::mat4(1.0f), camPos * -1.0f);
	view = glm::rotate(view, glm::radians(camRot.x), glm::vec3(1.f, 0.f, 0.f));
	view = glm::rotate(view, glm::radians(camRot.y), glm::vec3(0.f, 1.f, 0.f));
	view = glm::rotate(view, glm::radians(camRot.z), glm::vec3(0.f, 0.f, 1.f));
	
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), viewport.x / viewport.y, 0.01f, 1000.0f);

	ret[0] = model;
	ret[1] = view;
	ret[2] = proj;

	return ret;
}

float* mat4ToRaw(glm::mat4 mat)
{
	float* ret = new float[4*4];
	for (int i = 0; i < 4 * 4; i += 4)
	{
		// TODO: could just memcpy this
		ret[i] = mat[i / 4].x;
		ret[i + 1] = mat[i / 4].y;
		ret[i + 2] = mat[i / 4].z;
		ret[i + 3] = mat[i / 4].w;
	}

	return ret;
}

void drawMesh(Pinball::GameObject& obj, glm::vec2 viewport, GLuint vao, GLuint vbo, GLuint ibo, GLuint shader, glm::vec3 camPos, glm::vec3 camRot, Pinball::Light sunLight, std::vector<Pinball::Light> pointLights)
{
	float* verts = obj.Geometry().GetData();
	unsigned int* indices = obj.Geometry().GetIndices();

	glUseProgram(shader);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Position data
	glBufferData(GL_ARRAY_BUFFER, obj.Geometry().GetCount() * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(0); // enable position data
	// Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(1); // enable normal data
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glm::mat4* mvp = getTransform(obj, camPos, camRot, viewport);
	float* model = mat4ToRaw(mvp[0]), *view = mat4ToRaw(mvp[1]), *proj = mat4ToRaw(mvp[2]);
	
	glUniformMatrix4fv(glGetUniformLocation(shader, "_Model"), 1, false, model);
	glUniformMatrix4fv(glGetUniformLocation(shader, "_View"), 1, false, view);
	glUniformMatrix4fv(glGetUniformLocation(shader, "_Proj"), 1, false, proj);
	float* color = obj.Geometry().Color();
	glUniform3fv(glGetUniformLocation(shader, "_Color"), 1, color);

	float* sunDir = sunLight.Dir();
	float* sunCol = sunLight.Color();
	glUniform3fv(glGetUniformLocation(shader, "_Sun.direction"), 1, sunDir);
	glUniform3fv(glGetUniformLocation(shader, "_Sun.color"), 1, sunCol);

	for (size_t i = 0; i < pointLights.size(); i++)
	{
		float* plPos = pointLights[i].PointPos();
		float* plCol = pointLights[i].Color();

		glUniform3fv(glGetUniformLocation(shader, (std::string("_Lights[") + std::to_string(i) + "].position").c_str()), 1, plPos);
		glUniform3fv(glGetUniformLocation(shader, (std::string("_Lights[") + std::to_string(i) + "].color").c_str()), 1, plCol);
		glUniform1f(glGetUniformLocation(shader, (std::string("_Lights[") + std::to_string(i) + "].kc").c_str()), pointLights[i].kc);
		glUniform1f(glGetUniformLocation(shader, (std::string("_Lights[") + std::to_string(i) + "].kl").c_str()), pointLights[i].kl);
		glUniform1f(glGetUniformLocation(shader, (std::string("_Lights[") + std::to_string(i) + "].kq").c_str()), pointLights[i].kq);

		delete[] plCol;
		delete[] plPos;
	}

	glUniform1i(glGetUniformLocation(shader, "_LightCount"), pointLights.size());

	/*if (obj.Geometry().IsIndexed())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.Geometry().GetIndexCount() * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, obj.Geometry().GetIndexCount(), GL_UNSIGNED_INT, (GLvoid*)0);
	}
	else
	{*/
		glDrawArrays(GL_TRIANGLES, 0, obj.Geometry().GetCount());
	//}
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Release memory
	delete[] mvp;
	delete[] model;
	delete[] view;
	delete[] proj;
	delete[] verts;
	delete[] sunDir;
	delete[] sunCol;
}

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
	// GLFW initialisiation
	glfwInit();

	// Set created contexts to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Window creation
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Pinball Game", nullptr, nullptr);

	glfwShowWindow(window);

	// Assign GL context before working with GL
	glfwMakeContextCurrent(window);

	// OpenGL resources
	GLuint vbo, ibo;
	GLuint vao;
	GLuint diffuseShader, unlitShader;

	glewExperimental = true;
	// GLEW intialisation (has to be done once GL context is assigned)
	glewInit();

	glViewport(0, 0, 1280, 720);
	bool running = true;

	// Z-testing
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

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

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);

	// Shaders
	GLuint vertShader, fragShader;
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string vertShaderSrc = getFileContents("GLSL/Diffuse.vert");
	const char* vertShaderCStr = vertShaderSrc.c_str();
	int vertShaderLength = vertShaderSrc.length();
	glShaderSource(vertShader, 1, &vertShaderCStr, &vertShaderLength);
	std::string fragShaderSrc = getFileContents("GLSL/Diffuse.frag");
	const char* fragShaderCStr = fragShaderSrc.c_str();
	int fragShaderLength = fragShaderSrc.length();
	glShaderSource(fragShader, 1, &fragShaderCStr, &fragShaderLength);

	diffuseShader = glCreateProgram();
	glAttachShader(diffuseShader, vertShader);
	glAttachShader(diffuseShader, fragShader);
	glCompileShader(vertShader);
	glCompileShader(fragShader);
	glLinkProgram(diffuseShader);

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	vertShaderSrc = getFileContents("GLSL/Unlit.vert");
	vertShaderCStr = vertShaderSrc.c_str();
	vertShaderLength = vertShaderSrc.length();
	glShaderSource(vertShader, 1, &vertShaderCStr, &vertShaderLength);
	fragShaderSrc = getFileContents("GLSL/Unlit.frag");
	fragShaderCStr = fragShaderSrc.c_str();
	fragShaderLength = fragShaderSrc.length();
	glShaderSource(fragShader, 1, &fragShaderCStr, &fragShaderLength);

	unlitShader = glCreateProgram();
	glAttachShader(unlitShader, vertShader);
	glAttachShader(unlitShader, fragShader);
	glCompileShader(vertShader);
	glCompileShader(fragShader);
	glLinkProgram(unlitShader);

	double deltaTime = 0.0;
	double elapsedTime = glfwGetTime();

	bool paused = false;

	glm::vec3 camRot(60.0f, 0.0f, 0.0f);
	glm::vec3 camPos(0.0f, -3.3f, 25.0f);
	glm::vec3 lightPos(15.0f, 50.0f, -30.0f);
	Pinball::Light sunLight(glm::vec3(), glm::vec3(10.0f, -5.0f, 7.5f), glm::vec3(1.0, 1.0, 1.0));
	//light.pointPos = lightPos;
	sunLight.dir = glm::vec3(10.0f, -5.0f, 7.5f);

	// Prepare lights
	std::vector<Pinball::Light> pointLights = {
		Pinball::Light(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)),
		Pinball::Light(glm::vec3(12.5f, 10.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 1.0f, 0.0f)),
		Pinball::Light(glm::vec3(-12.5f, 10.0f, 10.0f), glm::vec3(), glm::vec3(1.0f, 0.0f, 1.0f)),
		Pinball::Light(glm::vec3(-12.5f, 10.0f, -10.0f), glm::vec3(), glm::vec3(0.0f, 1.0f, 1.0f)),
		Pinball::Light(glm::vec3(12.5f, 10.0f, -10.0f), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f))
	};

	float launchStrength = 0.0f;
	bool buildUp = false;

	while (running)
	{
		bool spacePressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			spacePressed = true;
		}

		// Process events
		glfwPollEvents();
		if (glfwWindowShouldClose(window))
		{
			running = false;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && spacePressed)
		{
			paused = !paused;
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, -20.0f));
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
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

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)ballObj.GetPxActor())->addForce(physx::PxVec3(0.f, 0.0f, 10.0f));
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setLinearVelocity(physx::PxVec3(-1.5f, 0.f, -1.f) * 50.f);
		}
		else
		{
			((physx::PxRigidDynamic*)levelObjects["FlipperR"].GetPxActor())->setLinearVelocity(physx::PxVec3(1.5f, 0.f, -1.f) * -50.f);
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		{
			launchStrength += 20.0f;
			buildUp = true;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE && buildUp)
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
		pointLights[0].pointPos = glm::vec3(bp.x, bp.y + 10.0f, bp.z);

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glPointSize(10.0f);
		int vWidth = 0, vHeight = 0;
		glfwGetWindowSize(window, &vWidth, &vHeight);
		drawMesh(boxObj, glm::vec2(vWidth, vHeight), vao, vbo, ibo, diffuseShader, camPos, camRot, sunLight, pointLights);
		for (std::map<std::string, Pinball::GameObject>::iterator it = levelObjects.begin(); it != levelObjects.end(); it++)
		{
			drawMesh(it->second, glm::vec2(vWidth, vHeight), vao, vbo, ibo, diffuseShader, camPos, camRot, sunLight, pointLights);
		}
		//drawMesh(planeObj, glm::vec2(vWidth, vHeight), vao, vbo, ibo, unlitShader);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	scene->release();

	PxCloseExtensions();

	return 0;
}