#pragma once

#include "Mesh.h"

namespace Pinball
{
	class GameObject
	{
	private:
		Mesh mMesh;

		physx::PxActor* mActor;
		std::vector<physx::PxShape*> mShapes;

		std::string mName;

		// Default material
		static physx::PxMaterial* _Mat;
		static bool _CreatedMat;
	public:
		enum Type { Dynamic = 0, Static };
		enum ColliderType { Trigger = 0, Collider, ColliderTrigger };

		GameObject();
		GameObject(Mesh geometry, Type actorType = Type::Dynamic, std::string name = "", ColliderType colliderType = ColliderType::Collider);
		Mesh Geometry();
		void Geometry(Mesh mesh, Type actorType = Type::Dynamic, ColliderType colliderType = ColliderType::Collider);
		physx::PxActor* GetPxActor();
		physx::PxRigidActor* GetPxRigidActor();
		std::string Name();
		void Name(std::string name);
		physx::PxTransform Transform();
		void Transform(physx::PxTransform transform);
	};
}