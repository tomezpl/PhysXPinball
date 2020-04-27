#pragma once

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
#include <glm/gtc/quaternion.hpp>

#include "GameObject.h"
#include "Camera.h"
#include "Light.h"
#include "Util.h"

namespace Pinball
{
	class Renderer 
	{
	protected:
		// Buffers
		unsigned int mVAO, mVBO, mIBO;

		// Window
		GLFWwindow* mWindow;
		// Window properties
		unsigned int mWidth, mHeight;

		// Prevents Init() from executing more than once
		static bool mInitialised;

		// Currently used shader
		unsigned int mCurrentShader;

		// Creates model, view & projection matrices for transformation
		glm::mat4* getTransform(GameObject& object, Camera camera);
	public:
		static void Init();

		Renderer(std::string name = "Renderer", int width = 1280, int height = 720);

		void Create(std::string name, int width, int height);

		void Draw(GameObject& object, Camera camera, std::vector<Light> lights, GLuint* shader = nullptr);

		void DrawParticle(GameObject& object, Camera camera, GLuint* shader = nullptr);
		
		GLFWwindow* Window();

		static unsigned int compileShader(std::string vertexShaderSource, std::string fragmentShaderSource);
	};
}