#include "Particle.h"
#include <random>

using namespace Pinball;

Mesh* Particle::mSparkMesh = nullptr;

Particle::Particle(physx::PxCooking* cooking, physx::PxVec3 origin, ParticleType type)
{
	mType = type;
	switch (mType)
	{
	case ParticleType::ePARTICLE_SPARK:
		mDuration = 0.33f;
		if (mSparkMesh == nullptr)
		{
			mSparkMesh = new Mesh(Mesh::createSphere(cooking, 0.125f));
		}
		Geometry(*mSparkMesh);
		SetupFiltering(FilterGroup::ePARTICLE, FilterGroup::eFLIPPER | FilterGroup::eFLOOR | FilterGroup::eTABLE);
	}

	Transform(physx::PxTransform(origin));

	mTimeLived = 0.0f;
	mKill = false;

	mFirstFrame = true;
}

bool Particle::IsAlive()
{
	return !mKill;
}

float Particle::LifeDuration()
{
	return mDuration;
}

float Particle::TimeLived()
{
	return mTimeLived;
}

void Particle::Advance(float deltaTime)
{
	mTimeLived += deltaTime;
	if (mTimeLived > mDuration)
	{
		mKill = true;
	}
	if (mFirstFrame)
	{
		float randomX = (float)rand() / RAND_MAX;
		float randomY = (float)rand() / RAND_MAX;
		float randomZ = (float)rand() / RAND_MAX;
		float strength = 50.0f;

		((physx::PxRigidDynamic*)GetPxActor())->addForce(physx::PxVec3(randomX, randomY, randomZ) * strength);
	}

	mFirstFrame = false;
}

Particle::~Particle()
{
	destroy();
}