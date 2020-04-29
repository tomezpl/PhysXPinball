#pragma once

#include "GameObject.h"
#include "Particle.h"

namespace Pinball {
	class Level {
	private:
		// Objects in this level:
		GameObject
			// Flippers
			*mFlipperL, *mFlipperR, // 0 1
			// Hinges for flippers
			*mHingeL, *mHingeR, // 2 3
			// Ramp (boosts the ball when in persistent contact)
			*mRamp, // 4
			// Table (play area)
			*mTable, // 5
			// Ball
			*mBall, // 6
			// Floor (separate from Table)
			*mFloor, // 7

			// Bumpers: 3 circular bumpers in a triangle formation at the top of the table
			*mBumper1, *mBumper2, *mBumper3, // 8 9 10
			// + two bumpers above each flipper (BumperL and BumperR)
			*mBumperL, *mBumperR, // 11 12
			// + two bumpers put in bottom corners of the table to act as "last resort" (BumperBL and BumperBR)
			*mBumperBL, *mBumperBR; // 13 14

		// Flipper joints
		physx::PxSphericalJoint* mFlipperJointL, *mFlipperJointR;

		// Particles in this level
		// Be careful when iterating over this vector, as dead particles will be deleted and marked with a nullptr, 
		// but will still remain inbetween live particles in this vector until they are replaced by new ones.
		// This means that the more often the particles are spawned (and the more of them at once), the bigger memory consumption for this will be.
		// The vector size thus doesn't represent how many live (valid) particles there are.
		// NbParticles() returns the number of valid particles.
		// ParticleAt(i) can return the particle given 0 <= i < NbParticles()
		std::vector<Particle*> mParticles;

		physx::PxScene* mScenePtr;
		
		void init();
	public:
		GameObject* const FlipperL();
		GameObject* const FlipperR();
		GameObject* const HingeL();
		GameObject* const HingeR();
		GameObject* const Ramp();
		GameObject* const Table();
		GameObject* const Ball();
		GameObject* const Floor();
		GameObject* const Bumper(int index);
		GameObject* const BumperL();
		GameObject* const BumperR();
		GameObject* const BumperBL();
		GameObject* const BumperBR();

		// Returns PxActors for all objects
		// Useful for adding to scene in one call
		physx::PxActor* const* AllActors();
		
		// Number of actors
		size_t NbActors();
		// Number of live particles
		size_t NbParticles();

		// Returns GameObject at specified index
		GameObject* const At(size_t index);
		// Returns actor at specified index
		physx::PxActor* const ActorAt(size_t index);

		// Returns particle at specified index (keep in mind killed particles will be skipped)
		Particle* const ParticleAt(size_t index);

		// Updates particles' state and removes them from scene if necessary
		void UpdateParticles(float deltaTime);

		// Emits a particle
		void SpawnParticle(physx::PxCooking* cooking, ParticleType type, physx::PxVec3 origin);

		// Emits multiple particles
		void SpawnParticles(physx::PxCooking* cooking, size_t count, ParticleType type, physx::PxVec3 origin);

		void SetScene(physx::PxScene* scenePtr);

		Level();
		Level(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking);
		void Load(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking);

		~Level();
	};
}