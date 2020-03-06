#pragma once

namespace Pinball {
	// Vertex format:
	// position: 3*float
	// normals: 3*float
	class Vertex
	{
	private:
		float* mData;
	public:
		// Copies vertices to a new float array.
		float* GetData();

		// Getters/setters for position and normal elements.
		float pX(), pX(float), pY(), pY(float), pZ(), pZ(float), nX(), nX(float), nY(), nY(float), nZ(), nZ(float);

		Vertex(float px = 0.0f, float py = 0.0f, float pz = 0.0f, float nx = 0.0f, float ny = 0.0f, float nz = 0.0f);
	};
}