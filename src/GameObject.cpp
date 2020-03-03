#include "GameObject.h"

Pinball::GameObject::GameObject()
{
	mActor = nullptr;
}

Pinball::GameObject::GameObject(Pinball::Mesh geometry, Pinball::GameObject::Type type)
{
	mActor = nullptr;

	Geometry(geometry, type);
}

Pinball::Mesh Pinball::GameObject::Geometry()
{
	return mMesh;
}

void Pinball::GameObject::Geometry(Pinball::Mesh mesh, Pinball::GameObject::Type type)
{
	mMesh = mesh;

	if (type == Pinball::GameObject::Type::Static)
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidStatic(physx::PxTransform(physx::PxIdentity));
	}
	else
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidDynamic(physx::PxTransform(physx::PxIdentity));
	}
}

physx::PxActor* Pinball::GameObject::GetPxActor()
{
	return mActor;
}

physx::PxTransform Pinball::GameObject::Transform()
{
	return ((physx::PxRigidActor*)mActor)->getGlobalPose();
}

void Pinball::GameObject::Transform(physx::PxTransform transform)
{
	((physx::PxRigidActor*)mActor)->setGlobalPose(transform);
}
