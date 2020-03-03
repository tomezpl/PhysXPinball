#include "GameObject.h"

using namespace Pinball;

GameObject::GameObject()
{
	mActor = nullptr;
}

GameObject::GameObject(Mesh geometry, GameObject::Type type)
{
	mActor = nullptr;

	Geometry(geometry, type);
}

Mesh GameObject::Geometry()
{
	return mMesh;
}

void GameObject::Geometry(Mesh mesh, GameObject::Type type)
{
	mMesh = mesh;

	if (type == GameObject::Type::Static)
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidStatic(physx::PxTransform(physx::PxIdentity));
	}
	else
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidDynamic(physx::PxTransform(physx::PxIdentity));
	}
}

physx::PxActor* GameObject::GetPxActor()
{
	return mActor;
}

physx::PxTransform GameObject::Transform()
{
	return ((physx::PxRigidActor*)mActor)->getGlobalPose();
}

void GameObject::Transform(physx::PxTransform transform)
{
	((physx::PxRigidActor*)mActor)->setGlobalPose(transform);
}
