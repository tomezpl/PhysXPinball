#pragma once

#include "Vertex.h"
#include <PxPhysicsAPI.h>
#include <vector>
#include <string>

namespace Pinball
{
	class Mesh
	{
	private:
		std::vector<Vertex> mVertices;
		std::vector<Vertex> mAllVerts;
		std::vector<unsigned int> mIndices;
		physx::PxGeometry* mPxGeometry;
		float* mColor;
		
		// Only used for primitive meshes
		// Half-extents of the primitive
		physx::PxVec3 mPrimitiveHx;

		int mType;

		std::string mName;
	public:
		enum MeshType { Plane = 0, Box, Sphere, Convex, TriangleList };

		// Creates the geometry for OpenGL and for PhysX. Appropriate meshType should be provided.
		void SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex, bool updatePx = true);
		void UpdatePx(physx::PxCooking* cooking);

		physx::PxVec3 GetCenterPoint();

		size_t GetIndexCount();
		bool IsIndexed();
		unsigned int* GetIndices();
		std::vector<unsigned int> GetIndexVector();

		// Returns vertex position data (3 floats per vertex, for PhysX)
		float* GetData();
		// Returns vertex position + normals (6 floats per vertex, for rendering)
		float* GetRenderData();
		std::vector<Vertex> GetDataVector();
		size_t GetCount();
		int GetMeshType();
		physx::PxGeometry* GetPxGeometry();
		Mesh();
		Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex, bool updatePx = true);
		void Color(float r, float g, float b);
		float* Color();
		std::string Name();
		void Name(std::string name);

		static Mesh createSphere(physx::PxCooking* cooking, float raidus = 1.0f, size_t stacks = 16, size_t slices = 8);
		static Mesh createBox(physx::PxCooking* cooking, float size = 1.0f);
		static Mesh createPlane(physx::PxCooking* cooking);
		static std::vector<Mesh> fromFile(std::string filePath, physx::PxCooking* cooking, bool updatePx = true);

		static std::vector<Vertex> reverseIndexing(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	};
}