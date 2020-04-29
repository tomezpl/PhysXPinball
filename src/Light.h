#pragma once
#include <glm/vec3.hpp>
namespace Pinball
{
	class Light
	{
		public:
			// constant, linear and quadratic term for point lights
			float kc = 1.0f, kl = 0.09f, kq = 0.032f;
			glm::vec3 color;
			glm::vec3 pointPos, dir;

			float* PointPos() { float* ret = new float[3]; ret[0] = pointPos.x; ret[1] = pointPos.y; ret[2] = pointPos.z; return ret; }
			float* Dir() { float* ret = new float[3]; ret[0] = dir.x; ret[1] = dir.y; ret[2] = dir.z; return ret; }
			float* Color() { float* ret = new float[3]; ret[0] = color.x; ret[1] = color.y; ret[2] = color.z; return ret; }

			Light(glm::vec3 position, glm::vec3 direction, glm::vec3 col) { color = col; pointPos = position; dir = direction; }
	};
}