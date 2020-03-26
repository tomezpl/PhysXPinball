#include "Mesh.h"
#include <iostream>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Util.h"

using namespace Pinball;

Mesh::Mesh()
{
	mPxGeometry = nullptr;
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
	mPrimitiveHx = physx::PxVec3(0.0f);
	mName = "";
}

Mesh::Mesh(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, Mesh::MeshType meshType, bool updatePx)
{
	mColor = new float[3]{ 0.0f, 0.0f, 0.0f };
	mPrimitiveHx = physx::PxVec3(0.0f);
	mName = "";
	SetVertices(vertices, cooking, indices, meshType, updatePx);
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

std::string Pinball::Mesh::Name()
{
	return mName;
}

void Pinball::Mesh::Name(std::string name)
{
	mName = name;
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
		}, cooking, {}, MeshType::Box);

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

std::vector<Mesh> Mesh::fromFile(std::string filePath, physx::PxCooking* cooking, bool updatePx)
{
	std::vector<Mesh> ret;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
	}

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		for (size_t j = 0; j < mesh->mNumVertices; j++)
		{
			vertices.push_back(Vertex(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z, mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z));
		}

		for (size_t j = 0; j < mesh->mNumFaces; j++)
		{
			indices.push_back(mesh->mFaces[j].mIndices[0]);
			indices.push_back(mesh->mFaces[j].mIndices[1]);
			indices.push_back(mesh->mFaces[j].mIndices[2]);
		}

		std::string meshName = std::string(scene->mRootNode->mChildren[i]->mName.C_Str());
		meshName = meshName.substr(0, meshName.find_last_of("_"));
		ret.push_back(Mesh(vertices, cooking, indices, meshName == "Ball" ? MeshType::Sphere : strContains(meshName, "Flipper") ? MeshType::Convex : MeshType::TriangleList));
		ret[ret.size()-1].Name(meshName);
	}

	return ret;
}

size_t Mesh::GetCount()
{
	return (IsIndexed()) ? mAllVerts.size() : mVertices.size();
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
		float* vert = (IsIndexed()) ? mAllVerts[i].GetData() : mVertices[i].GetData();
		float v0 = vert[0], v1 = vert[1], v2 = vert[2];
		ret[(i*3)] = vert[0];
		ret[(i * 3) + 1] = vert[1];
		ret[(i * 3) + 2] = vert[2];
		delete[] vert;
	}

	return ret;
}

// Assigns the vertex buffer and creates a convex PhysX mesh
void Mesh::SetVertices(std::vector<Vertex> vertices, physx::PxCooking* cooking, std::vector<unsigned int> indices, Mesh::MeshType meshType, bool updatePx)
{
	// TODO: shouldn't I delete each vertex first? they contain raw pointers
	mVertices = vertices;
	mAllVerts = reverseIndexing(vertices, indices);
	mIndices = indices;

	// Find sphere radius from vertices
	if (meshType == MeshType::Sphere || meshType == MeshType::Box)
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

	if (updatePx)
	{
		UpdatePx(cooking);
	}

}

void Mesh::UpdatePx(physx::PxCooking* cooking)
{
	physx::PxDefaultMemoryOutputStream buf;
	physx::PxDefaultMemoryOutputStream triBuf;

	physx::PxConvexMeshDesc meshDesc;
	physx::PxConvexMesh* convexMesh = nullptr;

	physx::PxTriangleMeshDesc triMeshDesc;
	physx::PxTriangleMesh* triMesh = nullptr;

	switch (mType)
	{
	case MeshType::Convex:
		meshDesc.points.count = GetCount();
		meshDesc.points.data = GetData();
		meshDesc.points.stride = sizeof(float) * 3; // Separate position coords from normals!

		if (IsIndexed())
		{
			// Align lowest index with 0
			/*unsigned int offset = *std::min_element(mIndices.begin(), mIndices.end());
			for (int i = 0; i < mIndices.size(); i++)
			{
				mIndices[i] -= offset;
			}*/
			meshDesc.indices.count = GetIndexCount();
			meshDesc.indices.data = GetIndices();
			meshDesc.indices.stride = 0;
		}
		if (!IsIndexed())
		{
			std::cout << std::endl;
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
		triMeshDesc.points.count = GetCount();
		triMeshDesc.points.data = GetData();
		triMeshDesc.points.stride = sizeof(float) * 3; // Separate position coords from normals!

		if (IsIndexed())
		{
			// Align lowest index with 0
			/*unsigned int offset = *std::min_element(mIndices.begin(), mIndices.end());
			for (int i = 0; i < mIndices.size(); i++)
			{
				mIndices[i] -= offset;
			}*/

			triMeshDesc.triangles.count = GetIndexCount() / 3;
			triMeshDesc.triangles.data = GetIndices();
			triMeshDesc.triangles.stride = sizeof(unsigned int) * 3;
			std::cout << "triMeshDesc valid: " << triMeshDesc.isValid() << std::endl;
		}
		if (!IsIndexed())
		{
			std::cout << std::endl;
		}

		if (cooking->cookTriangleMesh(triMeshDesc, triBuf))
		{
			std::cout << "Cooking PhysX triangle mesh successful." << std::endl;
		}
		else
		{
			std::cerr << "Cooking PhysX triangle mesh failed." << std::endl;
		}

		triMesh = PxGetPhysics().createTriangleMesh(*(new physx::PxDefaultMemoryInputData(triBuf.getData(), triBuf.getSize())));
		if (triMesh)
		{
			std::cout << "Created a PxTriangleMesh successfully." << std::endl;
		}
		else
		{
			std::cerr << "Failed to create PxTriangleMesh." << std::endl;
		}

		mPxGeometry = new physx::PxTriangleMeshGeometry(triMesh, physx::PxMeshScale());
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

physx::PxVec3 Pinball::Mesh::GetCenterPoint()
{
	physx::PxVec3 ret = physx::PxVec3(0.f);
	size_t count = GetCount();
	for (size_t i = 0; i < mVertices.size(); i++)
	{
		Vertex v = mVertices[i];
		ret += physx::PxVec3(v.pX(), v.pY(), v.pZ()) / count;
	}

	return ret;
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

std::vector<Vertex> Mesh::reverseIndexing(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
	std::vector<Vertex> ret;

	for (size_t i = 0; i < indices.size(); i++)
	{
		ret.push_back(vertices[indices[i]]);
	}

	return ret;
}