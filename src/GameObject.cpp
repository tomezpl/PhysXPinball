#include "GameObject.h"
#include "Middleware.h"
#include <glm/glm.hpp>

using namespace Pinball;

GameObject::GameObject()
{
	mActor = nullptr;
	mName = "";
}

GameObject::GameObject(Mesh& geometry, GameObject::Type type, float sf, float df, float cor, std::string name, GameObject::ColliderType colliderType)
{
	mActor = nullptr;
	mName = name;

	Geometry(geometry, type, sf, df, cor, colliderType);
}

Mesh GameObject::Geometry()
{
	return mMesh;
}

void GameObject::Geometry(Mesh& mesh, GameObject::Type type, float sf, float df, float cor, GameObject::ColliderType colliderType)
{
	mMesh = Mesh(mesh);

	physx::PxMeshScale scale = physx::PxMeshScale(mObjScale.mScale);

	switch (mMesh.GetPxGeometry()->getType())
	{
	case physx::PxGeometryType::eCONVEXMESH:
		((physx::PxConvexMeshGeometry*)mMesh.GetPxGeometry())->scale = scale;
	case physx::PxGeometryType::eTRIANGLEMESH:
		((physx::PxTriangleMeshGeometry*)mMesh.GetPxGeometry())->scale = scale;
	}

	mShapes = { PxGetPhysics().createShape(*mMesh.GetPxGeometry(), *PxGetPhysics().createMaterial(sf, df, cor), true) }; // TODO: won't this cause a memory leak on reinitialisation?

	if (mMesh.GetMeshType() == Mesh::MeshType::Plane)
	{
		mShapes[0]->setLocalPose(physx::PxTransform(0.0f, 0.0f, 0.0f, physx::PxQuat(glm::radians(90.0f), physx::PxVec3(0.0f, 0.0f, 1.0f))));
	}

	if (type == GameObject::Type::Static)
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidStatic(physx::PxTransform(physx::PxIdentity));
		
		for (int i = 0; i < mShapes.size(); i++)
		{
			((physx::PxRigidStatic*)mActor)->attachShape(*mShapes[i]);
		}
	}
	else
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidDynamic(physx::PxTransform(physx::PxIdentity));

		for (int i = 0; i < mShapes.size(); i++)
		{
			((physx::PxRigidDynamic*)mActor)->attachShape(*mShapes[i]);
		}
	}

	mActor->setName(mName.c_str());
}

physx::PxActor* GameObject::GetPxActor()
{
	return mActor;
}

physx::PxRigidActor* GameObject::GetPxRigidActor()
{
	return (physx::PxRigidActor*)mActor;
}

std::string GameObject::Name()
{
	return mName;
}

void GameObject::Name(std::string name)
{
	mName = name;

	const char* cName = new const char[name.length() + 1];
	memcpy((void*)cName, name.c_str(), name.length() * sizeof(const char));
	const char nullTerminator = '\0';
	memcpy((void*)(cName + name.length()), &nullTerminator, sizeof(const char));

	mActor->setName(cName);
}

physx::PxTransform GameObject::Transform()
{
	return ((physx::PxRigidActor*)mActor)->getGlobalPose();
}

void GameObject::Transform(physx::PxTransform transform)
{
	((physx::PxRigidActor*)mActor)->setGlobalPose(transform);
}

void GameObject::SetupFiltering(unsigned int filterGroup, unsigned int filterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // the FilterGroup this object identifies with
	filterData.word1 = filterMask; // the FilterGroup this object needs to collide with

	physx::PxRigidActor* actor = GetPxRigidActor();
	const physx::PxU32 numShapes = actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape * [numShapes];

	actor->getShapes(shapes, numShapes);

	// Set this filter data for all shapes of this object
	for (int i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}

	delete[] shapes;
}

ObjectScale& GameObject::Scale()
{
	return mObjScale;
}

void GameObject::destroy()
{
	for (size_t i = 0; i < mShapes.size(); i++)
	{
		if (mShapes[i] != nullptr && mShapes[i]->isReleasable())
		{
			if (mShapes[i]->getActor() != NULL)
			{
				mShapes[i]->getActor()->release();
			}
			//mShapes[i]->release();
		}
	}
}

GameObject::~GameObject()
{
	destroy();
}

ObjectScale::ObjectScale()
{
	mScale = physx::PxVec3(1.0f);
	mMeshScale = physx::PxMeshScale(mScale);
}

float ObjectScale::X()
{
	return mScale.x;
}

void ObjectScale::X(float x)
{
	mScale.x = x;
}

float ObjectScale::Y()
{
	return mScale.y;
}

void ObjectScale::Y(float y)
{
	mScale.y = y;
}

float ObjectScale::Z()
{
	return mScale.z;
}

void ObjectScale::Z(float z)
{
	mScale.z = z;
}