#pragma once

#include "Vertex.h"
#include <PxPhysicsAPI.h>
#include <vector>
#include <3rdparty/tiny_obj_loader.h>

namespace Pinball {
	class Mesh {
	private:
		std::vector<Vertex> mVertices;
		std::vector<unsigned int> mIndices;
		physx::PxGeometry* mPxGeometry;
		float* mColor;
		
		// Only used for primitive meshes
		// Half-extents of the primitive
		physx::PxVec3 mPrimitiveHx;

		int mType;
	public:
		enum MeshType { Plane = 0, Box, Sphere, Convex, TriangleList };

		// Creates the geometry for OpenGL and for PhysX. Appropriate meshType should be provided.
		void SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex);

		size_t GetIndexCount();
		bool IsIndexed();
		unsigned int* GetIndices();

		float* GetData();
		size_t GetCount();
		int GetMeshType();
		physx::PxGeometry* GetPxGeometry();
		Mesh();
		Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex);
		void Color(float r, float g, float b);
		float* Color();
		static Mesh createBox(physx::PxCooking* cooking, float size = 1.0f);
		static Mesh createPlane(physx::PxCooking* cooking);
		static std::vector<Mesh> fromFile(std::string filePath, physx::PxCooking* cooking);
	};
}