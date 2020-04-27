#pragma once

#include "Mesh.h"

namespace Pinball
{
	class GameObject; // forward decl

	class ObjectScale
	{
		friend class GameObject;
	private:
		physx::PxVec3 mScale;
		physx::PxMeshScale mMeshScale;
	public:
		ObjectScale();

		float X();
		void X(float x);
		float Y();
		void Y(float y);
		float Z();
		void Z(float z);
	};

	struct FilterGroup
	{
		enum Enum
		{
			eBALL = (1 << 0),
			eFLIPPER = (1 << 1),
			eTABLE = (1 << 2),
			eFLOOR = (1 << 3),
			ePARTICLE = (1 << 4),
		};
	};

	class GameObject
	{
	private:
		Mesh mMesh;

		physx::PxActor* mActor;
		std::vector<physx::PxShape*> mShapes;

		std::string mName;

		ObjectScale mObjScale;

		// Default material
		static physx::PxMaterial* _Mat;
		static bool _CreatedMat;
	protected:
		void destroy();
	public:
		enum Type { Dynamic = 0, Static };
		enum ColliderType { Trigger = 0, Collider, ColliderTrigger };

		GameObject();
		GameObject(Mesh geometry, Type actorType = Type::Dynamic, std::string name = "", ColliderType colliderType = ColliderType::Collider);
		Mesh Geometry();
		void Geometry(Mesh mesh, Type actorType = Type::Dynamic, ColliderType colliderType = ColliderType::Collider);
		physx::PxActor* GetPxActor();
		physx::PxRigidActor* GetPxRigidActor();
		std::string Name();
		void Name(std::string name);
		physx::PxTransform Transform();
		void Transform(physx::PxTransform transform);

		void SetupFiltering(unsigned int filterGroup, unsigned int filterMask);

		ObjectScale& Scale();

		~GameObject();
	};
}