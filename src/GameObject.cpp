#include "GameObject.h"
#include "Middleware.h"

using namespace Pinball;

physx::PxMaterial* GameObject::_Mat = nullptr;
bool GameObject::_CreatedMat = false;

GameObject::GameObject()
{
	mActor = nullptr;

	if (!_CreatedMat)
	{
		_Mat = PxGetPhysics().createMaterial(0.f, 0.f, 0.f);
		_CreatedMat = true;
	}
}

GameObject::GameObject(Mesh geometry, GameObject::Type type, GameObject::ColliderType colliderType)
{
	mActor = nullptr;

	if (!_CreatedMat)
	{
		_Mat = PxGetPhysics().createMaterial(0.f, 0.f, 0.f);
		_CreatedMat = true;
	}

	Geometry(geometry, type, colliderType);
}

Mesh GameObject::Geometry()
{
	return mMesh;
}

void GameObject::Geometry(Mesh mesh, GameObject::Type type, GameObject::ColliderType colliderType)
{
	mMesh = mesh;

	if (type == GameObject::Type::Static)
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidStatic(physx::PxTransform(physx::PxIdentity));
		((physx::PxRigidStatic*)mActor)->createShape(*Geometry().GetPxGeometry(), *_Mat);
	}
	else
	{
		mActor = (physx::PxActor*)PxGetPhysics().createRigidDynamic(physx::PxTransform(physx::PxIdentity));
		((physx::PxRigidDynamic*)mActor)->createShape(*Geometry().GetPxGeometry(), *_Mat);
	}

	// TODO: can this cause memory leaks on reinitialising geometry?
	Middleware::UserData* userData = new Middleware::UserData();
	userData->isTrigger = colliderType == ColliderType::Trigger || ColliderType::ColliderTrigger;
	userData->isCollider = colliderType == ColliderType::Collider || ColliderType::ColliderTrigger;

	mActor->userData = userData;
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
