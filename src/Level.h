#pragma once

#include "GameObject.h"

namespace Pinball {
	class Level {
	private:
		GameObject 
			*mFlipperL, *mFlipperR, // 0 1
			*mHingeL, *mHingeR, // 2 3
			*mRamp, // 4
			*mTable, // 5
			*mBall; // 6
		void init();
	public:
		GameObject* const FlipperL();
		GameObject* const FlipperR();
		GameObject* const HingeL();
		GameObject* const HingeR();
		GameObject* const Ramp();
		GameObject* const Table();
		GameObject* const Ball();

		// Returns PxActors for all objects
		// Useful for adding to scene in one call
		physx::PxActor* const* AllActors();
		// Number of actors
		size_t NbActors();

		// Returns GameObject at specified index
		GameObject* const At(size_t index);
		// Returns actor at specified index
		physx::PxActor* const ActorAt(size_t index);

		Level();
		Level(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking);
		void Load(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking);
	};
}