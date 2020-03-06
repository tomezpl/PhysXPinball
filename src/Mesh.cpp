#include "Mesh.h"
#include <iostream>

using namespace Pinball;

Mesh::Mesh()
{
	mPxGeometry = nullptr;
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
	mPrimitiveHx = physx::PxVec3(0.0f);
}

Mesh::Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, Mesh::MeshType meshType)
{
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
	mPrimitiveHx = physx::PxVec3(0.0f);
	SetVertices(vertices, cooking, indices, meshType);
}

void Mesh::Color(float r, float g, float b)
{
	mColor[0] = r;
	mColor[1] = g;
	mColor[2] = b;
}

float* Mesh::Color()
{
	return mColor;
}

Mesh Mesh::createBox(physx::PxCooking* cooking, float size)
{
	Mesh ret;
	float halfSize = size / 2.0f;
	ret.mPrimitiveHx = physx::PxVec3(halfSize);

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
		}, cooking, {});

	return ret;
}

Mesh Mesh::createPlane(physx::PxCooking* cooking)
{
	Mesh ret;
	float size = 10000.0f; // PhysX planes are infinite so the OpenGL plane should also cover as much as possible

	ret.SetVertices({
		Vertex(-size, 0.f, -size, -size, 1.f, -size),
		Vertex(-size, 0.f, size, -size, 1.f, size),
		Vertex(size, 0.f, size, size, 1.f, size),
		Vertex(size, 0.f, -size, size, 1.f, -size),
		Vertex(-size, 0.f, -size, -size, 1.f, -size),
		Vertex(size, 0.f, size, size, 1.f, size)
		}, cooking, {}, MeshType::Plane);
	return ret;
}

std::vector<Mesh> Mesh::fromFile(std::string filePath, physx::PxCooking* cooking)
{
	std::vector<Mesh> ret;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> loadedShapes;
	
	if (tinyobj::LoadObj(&attrib, &loadedShapes, nullptr, nullptr, nullptr, filePath.c_str()))
	{
		for (int i = 0; i < loadedShapes.size(); i++)
		{
			Mesh newMesh;
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			MeshType meshType = MeshType::Convex;

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < loadedShapes[i].mesh.num_face_vertices.size(); f++) {
				int fv = loadedShapes[i].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = loadedShapes[i].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					vertices.push_back(Vertex(vx, vy, vz, nx, ny, nz));
					indices.push_back(idx.vertex_index); // TODO: does this mean our normals might get lost?
				}
				index_offset += fv;
			}

			if (loadedShapes[i].name.find("Ball") != std::string::npos)
			{
				meshType = MeshType::Sphere;
			}

			newMesh.SetVertices(vertices, cooking, indices, meshType);

			ret.push_back(newMesh);
		}
	}
	else
	{
		std::cerr << "Mesh::fromFile: failed loading file" << std::endl;
	}

	return ret;
}

size_t Mesh::GetCount()
{
	return mVertices.size();
}

int Mesh::GetMeshType()
{
	return mType;
}

physx::PxGeometry* Mesh::GetPxGeometry()
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
void Mesh::SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, Mesh::MeshType meshType)
{
	// TODO: shouldn't I delete each vertex first? they contain raw pointers
	mVertices = vertices;
	mIndices = indices;

	if (meshType == MeshType::Sphere)
	{
		float maxX = 0.f;

		for (int i = 0; i < mVertices.size(); i++)
		{
			// Since it's a sphere we only need to check one axis to find radius
			if (abs(mVertices[i].pX()) > maxX)
			{
				maxX = abs(mVertices[i].pX());
			}
		}
		mPrimitiveHx = physx::PxVec3(maxX);
	}

	//meshDesc.vertexLimit = GetCount();

	mType = meshType;

	physx::PxDefaultMemoryOutputStream buf;

	physx::PxConvexMeshDesc meshDesc;
	physx::PxConvexMesh* convexMesh = nullptr;

	switch (meshType)
	{
	case MeshType::Convex:
		meshDesc.points.count = GetCount();
		meshDesc.points.data = GetData();
		meshDesc.points.stride = sizeof(float) * 3; // Separate position coords from normals!

		if (IsIndexed())
		{
			meshDesc.indices.count = GetIndexCount();
			meshDesc.indices.data = GetIndices();
			meshDesc.indices.stride = 0;
		}

		meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		if (cooking->cookConvexMesh(meshDesc, buf))
		{
			std::cout << "Cooking PhysX convex mesh successful." << std::endl;
		}
		else
		{
			std::cerr << "Cooking PhysX convex mesh failed." << std::endl;
		}

		convexMesh = PxGetPhysics().createConvexMesh(*(new physx::PxDefaultMemoryInputData(buf.getData(), buf.getSize())));
		if (convexMesh)
		{
			std::cout << "Created a PxConvexMesh successfully." << std::endl;
		}
		else
		{
			std::cerr << "Failed to create PxConvexMesh." << std::endl;
		}
		mPxGeometry = new physx::PxConvexMeshGeometry(convexMesh);
		return;
	case MeshType::TriangleList:
		// TODO
		return;
	case MeshType::Box:
		mPxGeometry = new physx::PxBoxGeometry(mPrimitiveHx);
		return;
	case MeshType::Plane:
		mPxGeometry = new physx::PxPlaneGeometry();
		return;
	case MeshType::Sphere:
		mPxGeometry = new physx::PxSphereGeometry(mPrimitiveHx.x);
		return;
	}
}

size_t Mesh::GetIndexCount()
{
	return mIndices.size();
}

bool Mesh::IsIndexed()
{
	return mIndices.size() > 0;
}

unsigned int* Mesh::GetIndices()
{
	return mIndices.data();
}
