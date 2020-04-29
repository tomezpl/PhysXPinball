#pragma once

#include "Mesh.h"

namespace Pinball
{
	class GameObject; // forward decl

	struct FilterGroup
	{
		enum Enum
		{
			eBALL = (1 << 0),
			eFLIPPER = (1 << 1),
			eTABLE = (1 << 2),
			eFLOOR = (1 << 3),
			ePARTICLE = (1 << 4),
			eBUMPER = (1 << 5),
		};
	};

	class GameObject
	{
	private:
		Mesh mMesh;

		physx::PxActor* mActor;
		std::vector<physx::PxShape*> mShapes;

		float mSf, mDf; // static & dynamic friction
		float mCOR; // coefficient of restitution

		std::string mName;
	protected:
		void destroy();
	public:
		enum Type { Dynamic = 0, Static };
		enum ColliderType { Trigger = 0, Collider, ColliderTrigger };

		GameObject();
		GameObject(Mesh& geometry, Type actorType = Type::Dynamic, float staticFriction = 0.f, float kineticFriction = 0.f, float restitution = 0.f, std::string name = "", ColliderType colliderType = ColliderType::Collider);

		Mesh Geometry();
		void Geometry(Mesh& mesh, Type actorType = Type::Dynamic, float staticFriction = 0.f, float kineticFriction = 0.f, float restitution = 0.f, ColliderType colliderType = ColliderType::Collider);

		physx::PxActor* GetPxActor();
		physx::PxRigidActor* GetPxRigidActor();

		std::string Name();
		void Name(std::string name);

		physx::PxTransform Transform();
		void Transform(physx::PxTransform transform);

		// Configure collision filtering. filterGroup is the object's FilterGroup, filterMask is a logical OR combination of collider FilterGroups.
		void SetupFiltering(unsigned int filterGroup, unsigned int filterMask);

		~GameObject();
	};
}