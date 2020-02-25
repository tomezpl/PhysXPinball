#include "Mesh.h"
#include <iostream>

using namespace Pinball;

Mesh::Mesh()
{
	mPxGeometry = nullptr;
}

Pinball::Mesh::Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking)
{
	SetVertices(vertices, cooking);
}

Mesh Pinball::Mesh::createBox(physx::PxCooking* cooking, float size)
{
	Mesh ret;

	ret.SetVertices({
		// Front wall
		Vertex(-1.f, -1.f, 1.f, -1.f, -1.f, 1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, -1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f, -1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f, -1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, -1.f, 1.f),

		// Bottom wall
		Vertex(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f),
		Vertex(-1.f, -1.f, 1.f, -1.f, -1.f, 1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, -1.f, 1.f),
		Vertex(1.f, -1.f, -1.f, 1.f, -1.f, -1.f),
		Vertex(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, -1.f, 1.f),

		// Back wall
		Vertex(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f),
		Vertex(1.f, -1.f, -1.f, 1.f, -1.f, -1.f),
		Vertex(-1.f, 1.f, -1.f, -1.f, 1.f, -1.f),
		Vertex(-1.f, 1.f, -1.f, -1.f, 1.f, -1.f),
		Vertex(1.f, 1.f, -1.f, 1.f, 1.f, -1.f),
		Vertex(1.f, -1.f, -1.f, 1.f, -1.f, -1.f),

		// Top wall
		Vertex(-1.f, 1.f, -1.f, -1.f, 1.f, -1.f),
		Vertex(-1.f, 1.f, 1.f, -1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, -1.f, 1.f, 1.f, -1.f),
		Vertex(-1.f, 1.f, -1.f, -1.f, 1.f, -1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f),

		// Left wall
		Vertex(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f),
		Vertex(-1.f, -1.f, 1.f, -1.f, -1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f, -1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f, -1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, -1.f, -1.f, 1.f, -1.f),
		Vertex(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f),

		// Right wall
		Vertex(1.f, -1.f, -1.f, 1.f, -1.f, -1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, -1.f, 1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, -1.f, 1.f, 1.f, -1.f),
		Vertex(1.f, -1.f, -1.f, 1.f, -1.f, -1.f)
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

void Mesh::SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking)
{
	// TODO: shouldn't I delete each vertex first? they contain raw pointers
	mVertices = vertices;


	physx::PxConvexMeshDesc meshDesc;
	meshDesc.points.count = GetCount();
	meshDesc.points.data = GetData();
	meshDesc.points.stride = sizeof(float) * 3;

	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
	meshDesc.vertexLimit = GetCount();

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