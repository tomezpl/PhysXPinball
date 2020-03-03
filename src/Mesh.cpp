#include "Mesh.h"
#include <iostream>

using namespace Pinball;

Mesh::Mesh()
{
	mPxGeometry = nullptr;
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
}

Pinball::Mesh::Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking)
{
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
	SetVertices(vertices, cooking);
}

void Pinball::Mesh::Color(float r, float g, float b)
{
	mColor[0] = r;
	mColor[1] = g;
	mColor[2] = b;
}

float* Pinball::Mesh::Color()
{
	return mColor;
}

Mesh Pinball::Mesh::createBox(physx::PxCooking* cooking, float size)
{
	Mesh ret;
	float halfSize = size / 2.0f;

	ret.SetVertices({
		// Front wall
		Vertex(-halfSize, -halfSize, halfSize, -halfSize, -halfSize, halfSize),
		Vertex(halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize),
		Vertex(-halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize),
		Vertex(-halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize),
		Vertex(halfSize, halfSize, halfSize, halfSize, halfSize, halfSize),
		Vertex(halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize),

		// Bottom wall
		Vertex(-halfSize, -halfSize, -halfSize, -halfSize, -halfSize, -halfSize),
		Vertex(-halfSize, -halfSize, halfSize, -halfSize, -halfSize, halfSize),
		Vertex(halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize),
		Vertex(halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize),
		Vertex(-halfSize, -halfSize, -halfSize, -halfSize, -halfSize, -halfSize),
		Vertex(halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize),

		// Back wall
		Vertex(-halfSize, -halfSize, -halfSize, -halfSize, -halfSize, -halfSize),
		Vertex(halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize),
		Vertex(-halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize),
		Vertex(-halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize),
		Vertex(halfSize, halfSize, -halfSize, halfSize, halfSize, -halfSize),
		Vertex(halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize),

		// Top wall
		Vertex(-halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize),
		Vertex(-halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize),
		Vertex(halfSize, halfSize, halfSize, halfSize, halfSize, halfSize),
		Vertex(halfSize, halfSize, -halfSize, halfSize, halfSize, -halfSize),
		Vertex(-halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize),
		Vertex(halfSize, halfSize, halfSize, halfSize, halfSize, halfSize),

		// Left wall
		Vertex(-halfSize, -halfSize, -halfSize, -halfSize, -halfSize, -halfSize),
		Vertex(-halfSize, -halfSize, halfSize, -halfSize, -halfSize, halfSize),
		Vertex(-halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize),
		Vertex(-halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize),
		Vertex(-halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize),
		Vertex(-halfSize, -halfSize, -halfSize, -halfSize, -halfSize, -halfSize),

		// Right wall
		Vertex(halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize),
		Vertex(halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize),
		Vertex(halfSize, halfSize, halfSize, halfSize, halfSize, halfSize),
		Vertex(halfSize, halfSize, halfSize, halfSize, halfSize, halfSize),
		Vertex(halfSize, halfSize, -halfSize, halfSize, halfSize, -halfSize),
		Vertex(halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize)
		}, cooking);

	return ret;
}

Mesh Pinball::Mesh::createPlane(physx::PxCooking* cooking, float size)
{
	Mesh ret;
	float halfSize = size / 2.0f;

	ret.SetVertices({
		Vertex(-halfSize, 0.f, -halfSize, -halfSize, 1.f, -halfSize),
		Vertex(-halfSize, 0.f, halfSize, -halfSize, 1.f, halfSize),
		Vertex(halfSize, 0.f, halfSize, halfSize, 1.f, halfSize),
		Vertex(halfSize, 0.f, -halfSize, halfSize, 1.f, -halfSize),
		Vertex(-halfSize, 0.f, -halfSize, -halfSize, 1.f, -halfSize),
		Vertex(halfSize, 0.f, halfSize, halfSize, 1.f, halfSize)
		}, cooking);

	return ret;
}

size_t Mesh::GetCount()
{
	return mVertices.size();
}

physx::PxGeometry* Pinball::Mesh::GetPxGeometry()
{
	return mPxGeometry;
}

// Returns position data
float* Mesh::GetData()
{
	size_t count = GetCount();
	float* ret = new float[count * 3];

	for (size_t i = 0; i < count; i++)
	{
		// TODO: this is bugged, using slow version for now 
		//memcpy(ret + i*3, mVertices[i].GetData(), 3 * sizeof(float));
		float* vert = mVertices[i].GetData();
		float v0 = vert[0], v1 = vert[1], v2 = vert[2];
		ret[(i*3)] = vert[0];
		ret[(i * 3) + 1] = vert[1];
		ret[(i * 3) + 2] = vert[2];
	}

	return ret;
}

// Assigns the vertex buffer and creates a convex PhysX mesh
void Mesh::SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking)
{
	// TODO: shouldn't I delete each vertex first? they contain raw pointers
	mVertices = vertices;


	physx::PxConvexMeshDesc meshDesc;
	meshDesc.points.count = GetCount();
	meshDesc.points.data = GetData();
	meshDesc.points.stride = sizeof(float) * 3;

	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
	//meshDesc.vertexLimit = GetCount();

	physx::PxDefaultMemoryOutputStream buf;
	if (cooking->cookConvexMesh(meshDesc, buf))
	{
		std::cout << "Cooking PhysX convex mesh successful." << std::endl;
	}
	else
	{
		std::cerr << "Cooking PhysX convex mesh failed." << std::endl;
	}

	physx::PxConvexMesh* convexMesh = PxGetPhysics().createConvexMesh(*(new physx::PxDefaultMemoryInputData(buf.getData(), buf.getSize())));
	if (convexMesh)
	{
		std::cout << "Created a PxConvexMesh successfully." << std::endl;
	}
	else
	{
		std::cerr << "Failed to create PxConvexMesh." << std::endl;
	}
	mPxGeometry = new physx::PxConvexMeshGeometry(convexMesh);
}