#pragma once

#include <foundation/PxTransform.h>

namespace Pinball
{
	class Camera
	{
	protected:
		physx::PxTransform mTransform;
	public:
		Camera(physx::PxTransform transform = physx::PxTransform(physx::PxIdentity));

		physx::PxVec3 Position();
		void Position(float x, float y, float z);
		void Position(physx::PxVec3 position);

		physx::PxQuat Orientation();
		void Orientation(float x, float y, float z, float w);
		void Orientation(physx::PxQuat orientation);
	};
}