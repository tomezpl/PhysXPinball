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

// stb_image
#include <3rdparty/stb_image.h>

#include "GameObject.h"
#include "Particle.h"
#include "Level.h"
#include "Camera.h"
#include "Light.h"
#include "Util.h"

namespace Pinball
{
	class Renderer;

	class Image {
		friend class Renderer;
	private:
		int mWidth, mHeight;
		int mSpectrum;
		unsigned char* mData;

		unsigned int mTexture; // OpenGL texture handle for this image
	public:
		Image();
		Image(std::string filePath);
		void Load(std::string filePath);
		unsigned char* GetData();

		// Returns OpenGL texture handle
		unsigned int& GetTexture();

		int DataLength();
		int Width();
		int Height();
	};

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

		// If drawing multiple particles of the same type, use DrawParticles, as it only copies the particle geometry once.
		// This assumes that ALL particles in the level are of the same type.
		void DrawParticles(Level& level, Camera camera, GLuint* shader = nullptr);

		void CreateTexture(Image& img);

		// Draw a 2D image somewhere on the screen
		void DrawImage(Image& image, float x, float y, float width, float height, GLuint* shader = nullptr);
		
		GLFWwindow* Window();

		static unsigned int compileShader(std::string vertexShaderSource, std::string fragmentShaderSource);
	};
}