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
		std::vector<Vertex> mVertices; // indexed vertices
		std::vector<Vertex> mAllVerts; // non-indexed vertices
		std::vector<unsigned int> mIndices; // indices

		// PhysX geometry holder
		physx::PxGeometry* mPxGeometry;

		// Mesh colour
		float* mColor;
		
		// Only used for primitive meshes
		// Half-extents of the primitive (or radius, if it's a sphere)
		physx::PxVec3 mPrimitiveHx;

		// Mesh type (trianglelist/primitive etc.)
		// Used for constructing PhysX geometry
		int mType;

		std::string mName;
	public:
		enum MeshType { Plane = 0, Box, Sphere, Convex, TriangleList };

		// Creates the geometry for OpenGL and for PhysX. Appropriate meshType should be provided.
		// indices must be an empty vector if the vertices are not indexed
		// updatePx decides whether PhysX geometry will also be (re)initialised
		void SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex, bool updatePx = true);

		// Initialise or reinitialise PhysX geometry
		void UpdatePx(physx::PxCooking* cooking);

		// Returns centre of all vertices in this Mesh.
		physx::PxVec3 GetCenterPoint();

		// Get number of indices
		size_t GetIndexCount();

		// Returns true if Mesh is indexed
		bool IsIndexed();

		// Get index buffer
		unsigned int* GetIndices();

		// Returns a copy of the index vector
		std::vector<unsigned int> GetIndexVector();

		// Raw vertex data. Returns vertex position data (3 floats per vertex, for PhysX)
		float* GetData();
		// Raw vertex data. Returns vertex position + normals (6 floats per vertex, for rendering)
		float* GetRenderData();

		// Returns structured Vertex data.
		std::vector<Vertex> GetDataVector();

		// Returns number of vertices
		size_t GetCount();

		// Returns type as MeshType
		int GetMeshType();

		// Returns PhysX representation of this Mesh
		physx::PxGeometry* GetPxGeometry();

		// Default constructor
		Mesh();

		// Construct with calling SetVertices. Refer to SetVertices.
		Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, MeshType meshType = MeshType::Convex, bool updatePx = true);

		// Set colour of this mesh
		void Color(float r, float g, float b);
		// Get colour of this mesh
		float* Color();

		// Get name of this mesh
		std::string Name();
		// Set name of this mesh
		void Name(std::string name);

		// Create a sphere of given complexity (stacks x slices) and radius
		static Mesh createSphere(physx::PxCooking* cooking, float raidus = 1.0f, size_t stacks = 16, size_t slices = 8);

		// Create a box with a given edge length
		static Mesh createBox(physx::PxCooking* cooking, float size = 1.0f);

		// Create an XZ plane
		static Mesh createPlane(physx::PxCooking* cooking);

		// Load mesh from file
		static std::vector<Mesh> fromFile(std::string filePath, physx::PxCooking* cooking, bool updatePx = true);

		// "Unindex" an indexed mesh (return all vertices, including duplicates)
		static std::vector<Vertex> reverseIndexing(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	};
}