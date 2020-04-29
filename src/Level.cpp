#include "Level.h"
#include "Util.h"

#include <map>

using namespace Pinball;

// Initialise pointers
void Level::init()
{
	if (!mFlipperL)
		mFlipperL = new GameObject();
	if (!mFlipperR)
		mFlipperR = new GameObject();
	if (!mHingeL)
		mHingeL = new GameObject();
	if (!mHingeR)
		mHingeR = new GameObject();

	if (!mRamp)
		mRamp = new GameObject();
	if (!mTable)
		mTable = new GameObject();
	if (!mBall)
		mBall = new GameObject();
	if (!mFloor)
		mFloor = new GameObject();

	if (!mBumper1)
		mBumper1 = new GameObject();
	if (!mBumper2)
		mBumper2 = new GameObject();
	if (!mBumper3)
		mBumper3 = new GameObject();
	if (!mBumperL)
		mBumperL = new GameObject();
	if (!mBumperR)
		mBumperR = new GameObject();
	if (!mBumperBL)
		mBumperBL = new GameObject();
	if (!mBumperBR)
		mBumperBR = new GameObject();

	mScenePtr = nullptr;
}

GameObject* const Level::FlipperL()
{
	return mFlipperL;
}

GameObject* const Level::FlipperR()
{
	return mFlipperR;
}

GameObject* const Level::HingeL()
{
	return mHingeL;
}

GameObject* const Level::HingeR()
{
	return mHingeR;
}

GameObject* const Level::Ramp()
{
	return mRamp;
}

GameObject* const Level::Table()
{
	return mTable;
}

GameObject* const Level::Ball()
{
	return mBall;
}

GameObject* const Level::Floor()
{
	return mFloor;
}

GameObject* const Level::Bumper(int i)
{
	switch (i)
	{
	case 1:
		return mBumper1;
	case 2:
		return mBumper2;
	case 3:
		return mBumper3;
	default:
		return nullptr;
	}
}

GameObject* const Level::BumperL()
{
	return mBumperL;
}

GameObject* const Level::BumperR()
{
	return mBumperR;
}

GameObject* const Level::BumperBL()
{
	return mBumperBL;
}

GameObject* const Level::BumperBR()
{
	return mBumperBR;
}

physx::PxActor* const* Level::AllActors()
{
	return new physx::PxActor * const [15]
	{
		mFlipperL->GetPxActor(),
		mFlipperR->GetPxActor(),
		mHingeL->GetPxActor(),
		mHingeR->GetPxActor(),
		mRamp->GetPxActor(),
		mTable->GetPxActor(),
		mBall->GetPxActor(),
		mFloor->GetPxActor(),
		mBumper1->GetPxActor(),
		mBumper2->GetPxActor(),
		mBumper3->GetPxActor(),
		mBumperL->GetPxActor(),
		mBumperR->GetPxActor(),
		mBumperBL->GetPxActor(),
		mBumperBR->GetPxActor()
	};
}

GameObject* const Level::At(size_t i)
{
	switch (i)
	{
	case 0:
		return mFlipperL;
	case 1:
		return mFlipperR;
	case 2:
		return mHingeL;
	case 3:
		return mHingeR;
	case 4:
		return mRamp;
	case 5:
		return mTable;
	case 6:
		return mBall;
	case 7:
		return mFloor;
	case 8:
		return mBumper1;
	case 9:
		return mBumper2;
	case 10:
		return mBumper3;
	case 11:
		return mBumperL;
	case 12:
		return mBumperR;
	case 13:
		return mBumperBL;
	case 14:
		return mBumperBR;
	default:
		return nullptr;
	}
}

physx::PxActor* const Level::ActorAt(size_t i)
{
	switch (i)
	{
	case 0:
		return mFlipperL->GetPxActor();
	case 1:
		return mFlipperR->GetPxActor();
	case 2:
		return mHingeL->GetPxActor();
	case 3:
		return mHingeR->GetPxActor();
	case 4:
		return mRamp->GetPxActor();
	case 5:
		return mTable->GetPxActor();
	case 6:
		return mBall->GetPxActor();
	case 7:
		return mFloor->GetPxActor();
	case 8:
		return mBumper1->GetPxActor();
	case 9:
		return mBumper2->GetPxActor();
	case 10:
		return mBumper3->GetPxActor();
	case 11:
		return mBumperL->GetPxActor();
	case 12:
		return mBumperR->GetPxActor();
	case 13:
		return mBumperBL->GetPxActor();
	case 14:
		return mBumperBR->GetPxActor();
	default:
		return nullptr;
	}
}

Particle* const Level::ParticleAt(size_t index)
{
	size_t i = 0;
	for (size_t j = 0; j < mParticles.size(); j++)
	{
		if(mParticles[j] == nullptr || !mParticles[j]->IsAlive())
		{
			continue;
		}
		else if (i == index)
		{
			return mParticles[j];
		}
		else
		{
			i++;
		}
	}

	return nullptr;
}

size_t Level::NbActors()
{
	return 15;
}

size_t Level::NbParticles()
{
	size_t ret = 0;
	for (size_t i = 0; i < mParticles.size(); i++)
	{
		if (mParticles[i] != nullptr && mParticles[i]->IsAlive())
		{
			ret++;
		}
	}
	return ret;
}

void Level::UpdateParticles(float dt)
{
	for (size_t i = 0; i < mParticles.size(); i++)
	{
		if (mParticles[i] != nullptr)
		{
			mParticles[i]->Advance(dt);
			if (!mParticles[i]->IsAlive())
			{
				mScenePtr->removeActor(*mParticles[i]->GetPxActor());
				delete mParticles[i];
				mParticles[i] = nullptr;
			}
		}
	}
}

void Level::SpawnParticle(physx::PxCooking* cooking, ParticleType type, physx::PxVec3 origin)
{
	bool added = false;
	for (size_t i = 0; i < mParticles.size(); i++)
	{
		if (mParticles[i] == nullptr)
		{
			mParticles[i] = new Particle(cooking, origin, type);
			mScenePtr->addActor(*mParticles[i]->GetPxActor());
			added = true;
			break;
		}
	}
	if (!added)
	{
		mParticles.push_back(new Particle(cooking, origin, type));
		mScenePtr->addActor(*mParticles[mParticles.size()-1]->GetPxActor());
		added = true;
	}
}

void Level::SpawnParticles(physx::PxCooking* cooking, size_t count, ParticleType type, physx::PxVec3 origin)
{
	physx::PxActor** particleActors = new physx::PxActor*[count];
	for (size_t i = 0; i < count; i++)
	{
		bool added = false;
		for (size_t j = 0; j < mParticles.size(); j++)
		{
			if (mParticles[j] == nullptr)
			{
				mParticles[j] = new Particle(cooking, origin, type);
				particleActors[i] = mParticles[j]->GetPxActor();
				added = true;
				break;
			}
		}
		if (!added)
		{
			mParticles.push_back(new Particle(cooking, origin, type));
			particleActors[i] = mParticles[mParticles.size() - 1]->GetPxActor();
			added = true;
		}
	}

	mScenePtr->addActors(particleActors, count);
}

void Level::SetScene(physx::PxScene* scenePtr)
{
	mScenePtr = scenePtr;
}

Level::Level()
{
	init();
}

Level::Level(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking)
{
	init();
	Load(meshFilePath, originFilePath, cooking);
}

void Level::Load(std::string meshFilePath, std::string originFilePath, physx::PxCooking* cooking)
{
	// Meshes for each object
	std::vector<Mesh> meshes = Mesh::fromFile(meshFilePath, cooking);

	// Origin points for each object
	std::vector<Mesh> originPoints = Mesh::fromFile(originFilePath, nullptr, false);
	std::map<std::string, Mesh> origins;
	for (size_t i = 0; i < originPoints.size(); i++)
	{
		origins[originPoints[i].Name()] = originPoints[i];
	}

	for (size_t i = 0; i < meshes.size(); i++)
	{
		GameObject::Type objType = GameObject::Static;
		GameObject* objToAssign = nullptr;
		std::string meshName = meshes[i].Name();
		if (strContains("BallFlipperLFlipperR", meshName))
		{
			objType = GameObject::Dynamic;
			if (strContains(meshName, "Ball"))
			{
				objToAssign = mBall;
				meshes[i].SetVertices(meshes[i].GetDataVector(), cooking, meshes[i].GetIndexVector(), Mesh::MeshType::Sphere);
			}
			else if (strContains(meshName, "FlipperL"))
			{
				objToAssign = mFlipperL;
			}
			else
			{
				objToAssign = mFlipperR;
			}
		}

		if (strContains(meshName, "Table"))
		{
			objToAssign = mTable;
		}
		else if (strContains(meshName, "Floor"))
		{
			objToAssign = mFloor;
		}
		else if (strContains(meshName, "Ramp"))
		{
			objToAssign = mRamp;
		}
		else if (strContains(meshName, "HingeL"))
		{
			objToAssign = mHingeL;
		}
		else if (strContains(meshName, "HingeR"))
		{
			objToAssign = mHingeR;
		}
		else if (strContains(meshName, "Bumper"))
		{
			if (strContains(meshName, "1"))
			{
				objToAssign = mBumper1;
			}
			else if (strContains(meshName, "2"))
			{
				objToAssign = mBumper2;
			}
			else if (strContains(meshName, "3"))
			{
				objToAssign = mBumper3;
			}
			else if (strContains(meshName, "BL"))
			{
				objToAssign = mBumperBL;
			}
			else if (strContains(meshName, "BR"))
			{
				objToAssign = mBumperBR;
			}
			else if (strContains(meshName, "L"))
			{
				objToAssign = mBumperL;
			}
			else if (strContains(meshName, "R"))
			{
				objToAssign = mBumperR;
			}
		}

		if (strContains("Ball FlipperL FlipperR HingeL HingeR Table Floor Ramp Bumper1 Bumper2 Bumper3 BumperL BumperR BumperBL BumperBR", meshName))
		{
			// physic material properties
			float sf = 0.0f, df = 0.0f, cor = 0.0f;
			
			// Set different properties for different types of objects
			if(strContains("Ball", meshName))
			{
				cor = 0.9f;
			}
			else if (strContains("Table", meshName))
			{
				cor = 0.3f;
				df = 0.4f;
				sf = 0.2f;
			}
			else if (strContains(meshName, "Bumper"))
			{
				cor = 1.0f;
			}

			objToAssign->Geometry(meshes[i], objType, sf, df, cor);

			// Set collision filtering flags
			if (strContains("Ball", meshName))
			{
				objToAssign->SetupFiltering(FilterGroup::eBALL, FilterGroup::eFLIPPER | FilterGroup::eFLOOR | FilterGroup::eTABLE | FilterGroup::eBUMPER);
			}
			else if (strContains("Table", meshName) || strContains("Ramp", meshName) || strContains(meshName, "Hinge"))
			{
				objToAssign->SetupFiltering(FilterGroup::eTABLE, FilterGroup::eBALL);
			}
			else if (strContains("Floor", meshName))
			{
				objToAssign->SetupFiltering(FilterGroup::eFLOOR, FilterGroup::eBALL);
			}
			else if (strContains(meshName, "Bumper"))
			{
				objToAssign->SetupFiltering(FilterGroup::eBUMPER, FilterGroup::eBALL);
			}
			else if (strContains(meshName, "Flipper"))
			{
				objToAssign->SetupFiltering(FilterGroup::eFLIPPER, FilterGroup::eBALL);
			}

			// Set colours
			if (strContains(meshName, "Ball"))
			{
				objToAssign->Geometry().Color(1.f, 1.f, 1.f);
			}
			else if (strContains(meshName, "Table") || strContains(meshName, "Ramp") || strContains(meshName, "Floor"))
			{
				objToAssign->Geometry().Color(193.f / 255.f, 154.f / 255.f, 107.f / 255.f);
			}
			else if (strContains(meshName, "BumperB") || strContains(meshName, "Hinge"))
			{
				objToAssign->Geometry().Color(193.f / 255.f * 0.75f, 154.f / 255.f * 0.75f, 107.f / 255.f * 0.75f);
			}
			else if (strContains(meshName, "BumperL") || strContains(meshName, "BumperR"))
			{
				objToAssign->Geometry().Color(0.75f, 0.f, 0.f);
			}
			else if (strContains(meshName, "Bumper"))
			{
				objToAssign->Geometry().Color(0.f, 0.33f, 0.66f);
			}
			else
			{
				objToAssign->Geometry().Color(0.5f, 0.5f, 0.5f);
			}

			if (objType == GameObject::Dynamic)
			{
				((physx::PxRigidDynamic*)objToAssign->GetPxActor())->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
			}
			objToAssign->Name(meshName);
			objToAssign->Transform(physx::PxTransform(origins[meshName].GetCenterPoint(), physx::PxQuat(physx::PxIdentity)));

		}
	}
}

Level::~Level()
{
	delete mFloor;
	delete mBall;
	delete mTable;
	delete mRamp;
	delete mHingeR;
	delete mHingeL;
	delete mFlipperR;
	delete mFlipperL;
}
