#pragma once

#include "Vertex.h"
#include <PxPhysicsAPI.h>
#include <vector>

namespace Pinball {
	class Mesh {
	private:
		std::vector<Vertex> mVertices;
		physx::PxGeometry* mPxGeometry;
	public:
		void SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking);
		float* GetData();
		size_t GetCount();
		physx::PxGeometry* GetPxGeometry();
		Mesh();
		Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking);
		static Mesh createBox(physx::PxCooking* cooking, float size = 1.0f);
	};
}