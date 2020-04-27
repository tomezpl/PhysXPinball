#include "Renderer.h"

using namespace Pinball;

bool Renderer::mInitialised = false;

Renderer::Renderer(std::string name, int w, int h)
{
	Init();
	Create(name, w, h);
}

void Renderer::Init()
{
	if (!mInitialised)
	{
		mInitialised = true;

		// GLFW initialisiation
		glfwInit();

		// Set created contexts to use OpenGL 3.3
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		glewExperimental = true;
	}
}

void Renderer::Create(std::string name, int width, int height)
{
	// Create window, store its size and set up GL context
	mWindow = glfwCreateWindow(mWidth = width, mHeight = height, name.c_str(), nullptr, nullptr);

	// Assign GL context before working with GL
	glfwMakeContextCurrent(mWindow);

	// GLEW intialisation (has to be done once GL context is assigned)
	glewInit();
	
	// Set viewport space
	glViewport(0, 0, mWidth, mHeight);

	// Enable Z-testing
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	// Generate buffers
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mIBO);
	glGenVertexArrays(1, &mVAO);

	// Display window
	glfwShowWindow(mWindow);
}

void Renderer::Draw(GameObject& obj, Camera cam, std::vector<Light> lights, GLuint* shader)
{
	// Retrieve raw vertex & index buffers from the GameObject
	float* verts = obj.Geometry().GetData();
	unsigned int* indices = obj.Geometry().GetIndices();

	// If a different shader has been provided than from the last draw call, change to that.
	if (shader != nullptr)
	{
		if (mCurrentShader != *shader)
		{
			glUseProgram(*shader);
			mCurrentShader = *shader;
		}
	}

	// Bind vertex array & buffer objects and feed with geometry data
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, obj.Geometry().GetCount() * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	// Vertex Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Vertex Normals (for lighting)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	// Unbind vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Get model, view & projection matrices
	glm::mat4* mvp = getTransform(obj, cam);
	float* model = mat4ToRaw(mvp[0]), * view = mat4ToRaw(mvp[1]), * proj = mat4ToRaw(mvp[2]);

	// Pass the transform matrices to shader
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_Model"), 1, false, model);
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_View"), 1, false, view);
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_Proj"), 1, false, proj);
	
	// Pass object colour to shader
	float* color = obj.Geometry().Color();
	glUniform3fv(glGetUniformLocation(mCurrentShader, "_Color"), 1, color);

	// Pass scene lighting to shader
	// Sun light
	float* sunDir = lights[0].Dir();
	float* sunCol = lights[0].Color();
	glUniform3fv(glGetUniformLocation(mCurrentShader, "_Sun.direction"), 1, sunDir);
	glUniform3fv(glGetUniformLocation(mCurrentShader, "_Sun.color"), 1, sunCol);
	// Point lights
	for (size_t i = 1; i < lights.size(); i++)
	{
		float* plPos = lights[i].PointPos();
		float* plCol = lights[i].Color();

		glUniform3fv(glGetUniformLocation(mCurrentShader, (std::string("_Lights[") + std::to_string(i-1) + "].position").c_str()), 1, plPos);
		glUniform3fv(glGetUniformLocation(mCurrentShader, (std::string("_Lights[") + std::to_string(i-1) + "].color").c_str()), 1, plCol);
		glUniform1f(glGetUniformLocation(mCurrentShader, (std::string("_Lights[") + std::to_string(i-1) + "].kc").c_str()), lights[i].kc);
		glUniform1f(glGetUniformLocation(mCurrentShader, (std::string("_Lights[") + std::to_string(i-1) + "].kl").c_str()), lights[i].kl);
		glUniform1f(glGetUniformLocation(mCurrentShader, (std::string("_Lights[") + std::to_string(i-1) + "].kq").c_str()), lights[i].kq);

		delete[] plCol;
		delete[] plPos;
	}

	// Set scene point light count. 
	// This must not be greater than the maximum number of point lights defined in the fragment shader (POINT_LIGHT_COUNT)
	glUniform1i(glGetUniformLocation(mCurrentShader, "_LightCount"), lights.size() - 1); // -1 as the sun light is in the same array but is a separate uniform

	// Non-indexed draw call for this object's triangles
	// Indexed draw exhibited severe flicker so all meshes are remapped to be unindexed on import (ie. contain duplicate triangles)
	glDrawArrays(GL_TRIANGLES, 0, obj.Geometry().GetCount());

	// Unbind vertex array object
	glBindVertexArray(0);

	// Cleanup to stop memory leaks
	delete[] mvp;
	delete[] model;
	delete[] view;
	delete[] proj;
	delete[] verts;
	delete[] sunDir;
	delete[] sunCol;
}

void Renderer::DrawParticle(GameObject& obj, Camera cam, GLuint* shader)
{
	// Retrieve raw vertex & index buffers from the GameObject
	float* verts = obj.Geometry().GetData();
	unsigned int* indices = obj.Geometry().GetIndices();

	// If a different shader has been provided than from the last draw call, change to that.
	if (shader != nullptr)
	{
		if (mCurrentShader != *shader)
		{
			glUseProgram(*shader);
			mCurrentShader = *shader;
		}
	}

	// Bind vertex array & buffer objects and feed with geometry data
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, obj.Geometry().GetCount() * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	// Vertex Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Unbind vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Get model, view & projection matrices
	glm::mat4* mvp = getTransform(obj, cam);
	float* model = mat4ToRaw(mvp[0]), * view = mat4ToRaw(mvp[1]), * proj = mat4ToRaw(mvp[2]);

	// Pass the transform matrices to shader
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_Model"), 1, false, model);
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_View"), 1, false, view);
	glUniformMatrix4fv(glGetUniformLocation(mCurrentShader, "_Proj"), 1, false, proj);

	// Non-indexed draw call for this object's triangles
	// Indexed draw exhibited severe flicker so all meshes are remapped to be unindexed on import (ie. contain duplicate triangles)
	glDrawArrays(GL_TRIANGLES, 0, obj.Geometry().GetCount());

	// Unbind vertex array object
	glBindVertexArray(0);

	// Cleanup to stop memory leaks
	delete[] mvp;
	delete[] model;
	delete[] view;
	delete[] proj;
	delete[] verts;
}

GLFWwindow* Renderer::Window()
{
	return mWindow;
}

unsigned int Renderer::compileShader(std::string vSource, std::string fSource)
{
	GLuint ret = glCreateProgram();
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER), fShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Add source code for vertex & fragment shaders
	glShaderSource(vShader, 1, new const char* (vSource.c_str()), new const GLint[1] { (int)vSource.length() });
	glShaderSource(fShader, 1, new const char* (fSource.c_str()), new const GLint[1] { (int)fSource.length() });

	// Attach vertex & fragment shader to the GL program object
	glAttachShader(ret, vShader);
	glAttachShader(ret, fShader);

	// Compile vertex & fragment shaders for this program object
	glCompileShader(vShader);
	glCompileShader(fShader);

	// Link the vertex & fragment shaders
	glLinkProgram(ret);

	return ret;
}

glm::mat4* Renderer::getTransform(GameObject& obj, Camera cam)
{
	glm::mat4* ret = new glm::mat4[3];

	physx::PxTransform worldTransform = obj.Transform();
	glm::vec3 modelPos = glm::vec3(worldTransform.p.x, worldTransform.p.y, worldTransform.p.z);
	glm::quat modelRot(worldTransform.q.w, worldTransform.q.x, worldTransform.q.y, worldTransform.q.z);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPos);
	model *= glm::mat4_cast(modelRot);
	model = glm::scale(model, glm::vec3(obj.Scale().X(), obj.Scale().Y(), obj.Scale().Z())); // TODO: no scaling for now

	physx::PxVec3 camPos = cam.Position();
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(camPos.x, camPos.y, camPos.z) * -1.0f);
	physx::PxQuat camOrient = cam.Orientation();
	view *= glm::mat4_cast(glm::quat(camOrient.w, camOrient.x, camOrient.y, camOrient.z));
	/*view = glm::rotate(view, glm::radians(camRot.x), glm::vec3(1.f, 0.f, 0.f));
	view = glm::rotate(view, glm::radians(camRot.y), glm::vec3(0.f, 1.f, 0.f));
	view = glm::rotate(view, glm::radians(camRot.z), glm::vec3(0.f, 0.f, 1.f));*/

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)mWidth / (float)mHeight, 0.01f, 1000.0f);

	ret[0] = model;
	ret[1] = view;
	ret[2] = proj;

	return ret;
}