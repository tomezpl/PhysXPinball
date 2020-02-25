#pragma once

#include "Mesh.h"

namespace Pinball {
	class GameObject {
	private:
		Mesh mMesh;

		// TODO: write a transform getter/setter
		physx::PxTransform mTransform;
		physx::PxActor* mActor;
	public:
		GameObject();
		GameObject(Mesh geometry);
		Mesh Geometry();
		void Geometry(Mesh mesh);
		physx::PxActor* GetPxActor();
	};
}