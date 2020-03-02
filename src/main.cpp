// C++ std libs
#include <vector>
#include <fstream>
#include <string>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>

// GL math
#include <glm/glm.hpp>
#include <glm/packing.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Game engine & PhysX
#include "GameObject.h"

glm::mat4 camera(float Translate, glm::vec2 const& Rotate)
{
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
	View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	return Projection * View * Model;
}

glm::mat4 transformMatrix(glm::vec3 camPos, Pinball::GameObject object)
{
	glm::mat4 proj = glm::perspective(glm::radians(60.f), 16.f / 9.f, 0.1f, 100.f);
	glm::mat4 view = glm::translate(glm::mat4(1.f), -camPos);
	// Model matrix doesn't take into account rotations relative to an anchor point/parent transform
	physx::PxTransform pose = ((physx::PxRigidActor*)object.GetPxActor())->getGlobalPose();
	glm::mat4 model = glm::rotate(glm::mat4(1.f), pose.q.w, glm::vec3(pose.q.x, pose.q.y, pose.q.z));
	model = glm::translate(model, glm::vec3(pose.p.x, pose.p.y, pose.p.z));
	model = glm::scale(model, glm::vec3(1.f));

	return proj * view * model;
}

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

	glewExperimental = true;
	// GLEW intialisation (has to be done once GL context is assigned)
	glewInit();

	glViewport(0, 0, 1280, 720);
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
	physx::PxFoundation* pxFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, pxAlloc, pxErrClb);
	PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale());
	physx::PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION, PxGetPhysics().getFoundation(), physx::PxCookingParams(physx::PxTolerancesScale()));

	Pinball::Mesh boxMesh = Pinball::Mesh::createBox(cooking);
	boxMesh.Color(0.0f, 1.0f, 0.0f);
	Pinball::GameObject boxObj(boxMesh);

	GLuint vbo;
	GLuint vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, boxObj.Geometry().GetCount() * 3 * sizeof(float), boxObj.Geometry().GetData(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	// Shaders
	GLuint vertShader, fragShader;
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string vertShaderSrc = getFileContents("GLSL/Unlit.vert");
	const char* vertShaderCStr = vertShaderSrc.c_str();
	int vertShaderLength = vertShaderSrc.length();
	glShaderSource(vertShader, 1, &vertShaderCStr, &vertShaderLength);
	std::string fragShaderSrc = getFileContents("GLSL/Unlit.frag");
	const char* fragShaderCStr = fragShaderSrc.c_str();
	int fragShaderLength = fragShaderSrc.length();
	glShaderSource(fragShader, 1, &fragShaderCStr, &fragShaderLength);

	GLuint program;
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glCompileShader(vertShader);
	glCompileShader(fragShader);
	glLinkProgram(program);

	//glBindAttribLocation(program, 0, "position");

	double deltaTime = 0.0;
	double elapsedTime = glfwGetTime();

	while (running)
	{
		// Process events
		glfwPollEvents();
		if (glfwWindowShouldClose(window))
		{
			running = false;
		}

		// Process logic, prepare scene
		double prevElapsedTime = elapsedTime;
		elapsedTime = glfwGetTime();
		deltaTime = elapsedTime - prevElapsedTime;

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glPointSize(10.0f);
		glUseProgram(program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glm::mat4 mvp = camera(5.f, glm::vec2(0.f, 5.f));
		float* mvpMat = new float[4*4];
		for (int i = 0; i < 4*4; i += 4)
		{
			mvpMat[i] = mvp[i / 4].x;
			mvpMat[i+1] = mvp[i / 4].y;
			mvpMat[i+2] = mvp[i / 4].z;
			mvpMat[i+3] = mvp[i / 4].w;
		}
		glUniformMatrix4fv(glGetUniformLocation(program, "_MVP"), 1, false, mvpMat);
		glUniform3fv(glGetUniformLocation(program, "_Color"), 1, boxMesh.Color());
		glDrawArrays(GL_TRIANGLES, 0, boxMesh.GetCount());
		glBindVertexArray(0);
		delete[] mvpMat;

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	return 0;
}