#pragma once

#include "GameObject.h"

namespace Pinball {
	enum ParticleType {
		ePARTICLE_SPARK = 0
	};

	class Particle : public GameObject
	{
	private:
		float mDuration;
		float mTimeLived;
		ParticleType mType;
		bool mKill; // should this particle be deleted?
		bool mFirstFrame; // is this the first frame of the particle's lifetime?

		// Cached meshes to reuse
		static Mesh* _SparkMesh;
	public:
		// Creates a particle of a given type at a given point in the scene.
		Particle(physx::PxCooking* cooking, physx::PxVec3 origin, ParticleType type);

		// Is the particle still alive? (valid)
		// This is mostly used internally in the Advance method, to identify when the Particle needs to be deleted.
		bool IsAlive();

		// Maximum lifespan
		float LifeDuration();

		// Time lived so far
		float TimeLived();

		// Update the particle's state after PhysX scene simulation step.
		void Advance(float deltaTime);

		~Particle();
	};
}