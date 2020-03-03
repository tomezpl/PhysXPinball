#pragma once

#include "Mesh.h"

namespace Pinball {
	class GameObject {
	private:
		Mesh mMesh;

		physx::PxActor* mActor;

		// Default material
		static physx::PxMaterial* _Mat;
		static bool _CreatedMat;
	public:
		enum Type { Dynamic = 0, Static };
		enum ColliderType { Trigger = 0, Collider, ColliderTrigger };

		GameObject();
		GameObject(Mesh geometry, Type actorType = Type::Dynamic, ColliderType colliderType = ColliderType::Collider);
		Mesh Geometry();
		void Geometry(Mesh mesh, Type actorType = Type::Dynamic, ColliderType colliderType = ColliderType::Collider);
		physx::PxActor* GetPxActor();
		physx::PxTransform Transform();
		void Transform(physx::PxTransform transform);
	};
}