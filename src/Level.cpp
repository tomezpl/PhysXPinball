#include "Level.h"
#include "Util.h"

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

physx::PxActor* const* Level::AllActors()
{
	return new physx::PxActor * const [8]
	{
		mFlipperL->GetPxActor(),
		mFlipperR->GetPxActor(),
		mHingeL->GetPxActor(),
		mHingeR->GetPxActor(),
		mRamp->GetPxActor(),
		mTable->GetPxActor(),
		mBall->GetPxActor(),
		mFloor->GetPxActor()
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
	default:
		return nullptr;
	}
}

size_t Level::NbActors()
{
	return 8;
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
	std::vector<Mesh> origins = Mesh::fromFile(originFilePath, nullptr, false);

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

		if (strContains("Ball FlipperL FlipperR HingeL HingeR Table Floor Ramp", meshName))
		{
			objToAssign->Geometry(meshes[i], objType);
			objToAssign->Name(meshName);
			objToAssign->Transform(physx::PxTransform(origins[i].GetCenterPoint(), physx::PxQuat(physx::PxIdentity)));
			objToAssign->Geometry().Color(0.5f, 0.5f, 0.5f);
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
