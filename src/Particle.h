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
	public:
		Particle(physx::PxCooking* cooking, physx::PxVec3 origin, ParticleType type);
		bool IsAlive();

		float LifeDuration();
		float TimeLived();
		void Advance(float deltaTime);
	};
}