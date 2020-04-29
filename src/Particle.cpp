#include "Particle.h"
#include <random>

using namespace Pinball;

Mesh* Particle::_SparkMesh = nullptr;

Particle::Particle(physx::PxCooking* cooking, physx::PxVec3 origin, ParticleType type)
{
	mType = type;
	switch (mType)
	{
	case ParticleType::ePARTICLE_SPARK:
		mDuration = 0.33f;

		// Initialise the spark mesh if it hasn't been initialised yet. It should be of fairly low complexity.
		if (_SparkMesh == nullptr)
		{
			_SparkMesh = new Mesh(Mesh::createSphere(cooking, 0.05f, 4, 2));
		}

		// Reuse the mesh
		Geometry(*_SparkMesh);

		// Disable collision for particles for better performance.
		SetupFiltering(FilterGroup::ePARTICLE, 0);
	}

	Transform(physx::PxTransform(origin));

	mTimeLived = 0.0f;
	mKill = false;

	// This will indicate that the particle is yet to have its initial force applied.
	// That force will be applied on the first call to Advance(dt)
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
		float strength = 500.0f; // initial kinetic energy

		((physx::PxRigidDynamic*)GetPxActor())->addForce(physx::PxVec3(randomX, randomY, randomZ) * strength);
	}

	mFirstFrame = false;
}

Particle::~Particle()
{
	destroy();
}