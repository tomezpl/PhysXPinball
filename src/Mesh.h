#pragma once

#include "Vertex.h"
#include <PxPhysicsAPI.h>
#include <vector>

namespace Pinball {
	class Mesh {
	private:
		std::vector<Vertex> mVertices;
		physx::PxGeometry* mPxGeometry;
		float* mColor;
	public:
		void SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking);
		float* GetData();
		size_t GetCount();
		physx::PxGeometry* GetPxGeometry();
		Mesh();
		Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking);
		void Color(float r, float g, float b);
		float* Color();
		static Mesh createBox(physx::PxCooking* cooking, float size = 1.0f);
		static Mesh createPlane(physx::PxCooking* cooking, float size = 1.0f);
	};
}