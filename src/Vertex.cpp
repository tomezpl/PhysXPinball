#include "Vertex.h"
#include <memory>

using namespace Pinball;

Vertex::Vertex(float px, float py, float pz, float nx, float ny, float nz)
{
	mData = new float[6]{ px,py,pz,nx,ny,nz };
}

float* Vertex::GetData()
{
	float* ret = new float[6];

	std::memcpy(ret, mData, 6 * sizeof(float));

	return ret;
}

float Vertex::pX() { return mData[0]; }
float Vertex::pX(float x) { return mData[0] = x; }

float Vertex::pY() { return mData[1]; }
float Vertex::pY(float y) { return mData[1] = y; }

float Vertex::pZ() { return mData[2]; }
float Vertex::pZ(float z) { return mData[2] = z; }

float Vertex::nX() { return mData[3]; }
float Vertex::nX(float x) { return mData[3] = x; }

float Vertex::nY() { return mData[4]; }
float Vertex::nY(float y) { return mData[4] = y; }

float Vertex::nZ() { return mData[5]; }
float Vertex::nZ(float z) { return mData[5] = z; }