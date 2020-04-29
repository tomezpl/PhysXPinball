#include "Camera.h"

using namespace Pinball;

Camera::Camera(physx::PxTransform transform)
{
	mTransform = transform;
}

physx::PxVec3 Camera::Position()
{
	return mTransform.p;
}

void Camera::Position(float x, float y, float z)
{
	mTransform.p.x = x;
	mTransform.p.y = y;
	mTransform.p.z = z;
}

void Camera::Position(physx::PxVec3 position)
{
	mTransform.p = position;
}

physx::PxQuat Camera::Orientation()
{
	return mTransform.q;
}

void Camera::Orientation(float x, float y, float z, float w)
{
	mTransform.q.x = x;
	mTransform.q.y = y;
	mTransform.q.z = z;
	mTransform.q.w = w;
}

void Camera::Orientation(physx::PxQuat orientation)
{
	mTransform.q = orientation;
}
