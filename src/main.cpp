#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>

int main(int* argc, char** argv)
{
	// GLFW initialisiation
	glfwInit();

	// Window creation
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Pinball Game", nullptr, nullptr);

	glfwShowWindow(window);

	// Assign GL context before working with GL
	glfwMakeContextCurrent(window);

	// GLEW intialisation (has to be done once GL context is assigned)
	glewInit();

	glViewport(0, 0, 1280, 720);
	bool running = true;

	float verts[] = { 0.5f, 0.5f, 1.f };

	GLuint vbo;
	GLuint vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);

	while (running)
	{
		// Process events
		glfwPollEvents();
		if (glfwWindowShouldClose(window))
		{
			running = false;
		}

		// Draw
		glClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glPointSize(10.0f);
		glDrawArrays(GL_POINTS, 0, 1);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	return 0;
}