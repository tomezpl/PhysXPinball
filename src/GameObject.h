#pragma once

#include "Mesh.h"

namespace Pinball {
	class GameObject {
	private:
		Mesh mMesh;

		physx::PxActor* mActor;
	public:
		enum Type { Dynamic = 0, Static };
		GameObject();
		GameObject(Mesh geometry, Type actorType = Type::Dynamic);
		Mesh Geometry();
		void Geometry(Mesh mesh, Type actorType = Type::Dynamic);
		physx::PxActor* GetPxActor();
		physx::PxTransform Transform();
		void Transform(physx::PxTransform transform);
	};
}