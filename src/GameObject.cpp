#include "GameObject.h"

Pinball::GameObject::GameObject()
{
	mActor = nullptr;
}

Pinball::GameObject::GameObject(Pinball::Mesh geometry)
{
	mActor = nullptr;
	mTransform = physx::PxTransform(physx::PxIdentity);

	Geometry(geometry);
}

Pinball::Mesh Pinball::GameObject::Geometry()
{
	return mMesh;
}

void Pinball::GameObject::Geometry(Pinball::Mesh mesh)
{
	mMesh = mesh;

	mActor = (physx::PxActor*)PxGetPhysics().createRigidDynamic(mTransform);
}

physx::PxActor* Pinball::GameObject::GetPxActor()
{
	return mActor;
}