#include "ModelImporter.h"

using namespace std;

using namespace Pinball;

// Default constructor
ModelImporter::ModelImporter()
{
	// Initialise filestreams to null
	mObjFile = nullptr;
	mMtlFile = nullptr;
}

// Opens file after constructor
ModelImporter::ModelImporter(string fn, string dir)
{
	// Initialise filestreams to null
	mObjFile = nullptr;
	mMtlFile = nullptr;

	// Open model file using the provided filename and directory path
	Init(fn, dir);
}

// Opens model file
bool ModelImporter::Init(string fn, string dir)
{
	// Combine directory path and filename together to get filepath
	fn = dir + "/" + fn;

	// Filename for the .obj file
	string objFN = fn;

	// Filename for the .mtl material library file
	string mtlFN = fn.substr(0, fn.length() - string(".obj").length()) + ".mtl";

	// Remove any existing geometry data from possible previous import operations using this instance
	mGeometry.clear();

	// Create filestreams
	mObjFile = new ifstream(objFN);
	mMtlFile = new ifstream(mtlFN);

	return true;
}

// Actual .obj parsing
bool ModelImporter::_ReadOBJ(physx::PxCooking* cooking)
{
	// Currently read line
	string line = "";

	// .obj indexes normals and assigns them to vertices, so they have to be loaded into a separate array first.
	// A separate array is also used for final set of vertices, as we ignore indexing; all vertices (duplicate or unique) are placed together, 
	// and normals are recalculated.
	std::vector<Vertex> verts, normals, uvs, finalVerts;

	vector<unsigned int> indices, finalIndices; // finalIndices will just be an array of indices with the size of n total vertices
	unsigned int indexCounter = 0; // TODO: an incrementing index to write for each vertex (as in, ignoring indexing altogether and just writing an index for each vertex). Might consider getting rid of drawing elements altogether and just draw arrays instead.

	Mesh::MeshType meshType = Mesh::TriangleList;

	while (mObjFile->good()) // check if file is still readable
	{
		getline(*mObjFile, line); // read next line
		if (line[0] == '#') // ignore comments
			continue;

		string keyword = ""; // current keyword, like "o" for object, "v" for vertex, etc.
		string data = ""; // current data, like coordinates when keyword is "v", or material name when keyword is "usemtl"
		int firstSpace = line.find(" "); // first whitespace character in the line
		keyword = line.substr(0, firstSpace); // skip till first whitespace
		std::string objectName = "";
		if (keyword == "o") // object (mesh) has been found
		{
			// Add last read mesh to model geometry
			if (verts.size() > 0)
			{
				meshType = Mesh::TriangleList;
				if (objectName.find("Ball") != std::string::npos)
				{
					meshType = Mesh::Sphere;
				}
				Mesh loadedMesh(finalVerts, cooking, finalIndices, meshType); // temporary mesh, don't index vertices
				mGeometry.push_back(loadedMesh); // add mesh to geometry set
			}
			objectName = line.substr(2);

			// Reset data & state to prepare for next mesh
			verts.clear();
			normals.clear();
			indices.clear();
			finalVerts.clear();
			finalIndices.clear();
			indexCounter = 0;
		}
		else if (keyword == "v" || keyword == "vn" || keyword == "vt") // a vertex coordinate (position, normal or texture uv) is found
		{
			Vertex v;
			data = line.substr(firstSpace + 1); // skip the keyword and first whitespace
			string xyz[3] = { "", "", "" }; // XYZ (or XY if keyword is vt) coordinates from the line
			for (int i = 0; i < 3 || (keyword == "vt" && i < 2); i++) // read all 3 (or 2 if vt) coordinates from the line
			{
				xyz[i] = data.substr(0, data.find(" "));
				data = data.substr(data.find(" ") + 1);
			}
			// Assign coordinates to the Vertex in accordance to coordinate type (pos/normal/uv)
			// TODO: Wasting memory here. In case of vt, there are 6 unused floats - that's 24 bytes per vertex. Should get rid of separate vectors too.
			if (keyword == "v")
			{
				v.pX(stof(xyz[0]));
				v.pY(stof(xyz[1]));
				v.pZ(stof(xyz[2]));
				verts.push_back(v);
			}
			else if (keyword == "vn")
			{
				v.nX(stof(xyz[0]));
				v.nY(stof(xyz[1]));
				v.nZ(stof(xyz[2]));
				normals.push_back(v);
			}
			/*else if (keyword == "vt")
			{
				v.s = stof(xyz[0]);
				v.t = stof(xyz[1]);
				uvs.push_back(v);
			}*/
		}
		else if (keyword == "f") // a face (triangle) has been found, this is where indices are taken from
		{
			data = line + " "; // current line + extra whitespace (a quick hack that will be used for loading normals further below, search "normalIndex")
			for (int i = 0; i < 3; i++) // The .obj file should be storing faces as triangles (NOT quads) for this to work
			{
				data = data.substr(data.find(" ") + 1); // skips "f " to start reading the integers
				string vertexIndex = data.substr(0, data.find("/")); // copy position index
				data = data.substr(data.find("/") + 1); // skips till after slash to read next index type
				// next up should be the vertex UV coord index, which might be empty
				string uvIndex = "";
				if (data.find("/") != 0) // UV coord index is NOT empty (e.g. `f 1/1/1`)
				{
					uvIndex = data.substr(0, data.find("/")); // copy uv index
					data = data.substr(data.find("/") + 1); // skips to next index type
				}
				else // UV coord index is empty (e.g. `f 1//1`)
					data = data.substr(1); // skips to next index type
				// next up is the vertex normal index, which is the last one for this vertex. The first two vertices for this face will be separated by a whitespace, while the last one ends with a newline, so that's why there was an extra whitespace added to data earlier.
				string normalIndex = data.substr(0, data.find(" "));

				// Convert all indices from strings to integers
				unsigned long long vIdx = stoll(vertexIndex);
				unsigned long long uvIdx = 0;
				if (uvIndex != "") // check if uv index was even found before converting
					uvIdx = stoll(uvIndex);
				unsigned long long nIdx = stoll(normalIndex);

				indices.push_back(indexCounter++); // add incrementing index

				// finalVerts will store each vertex (no matter if it's duplicated - it ignores indexing)
				// the indices are zero-based (hence the v/n/uvIdx - 1). indexCounter was just incremented hence -1
				// The reason why indexing code is even here is to find out which vertex is the normal/UV coordinate is associated with
				// the .obj file stores vertex positions, normals and uv separately yet they're linked together by indices
				// TODO: this is not the most efficient way of doing things (and certainly not an elegant one) but it works for now
				finalVerts.push_back(verts[vIdx - 1]);
				finalVerts[indexCounter - 1].nX(normals[nIdx - 1].nX());
				finalVerts[indexCounter - 1].nY(normals[nIdx - 1].nY());
				finalVerts[indexCounter - 1].nZ(normals[nIdx - 1].nZ());
				/*if (uvIdx != 0)
				{
					finalVerts[indexCounter - 1].s = uvs[uvIdx - 1].s;
					finalVerts[indexCounter - 1].t = uvs[uvIdx - 1].t;
				}*/
			}
		}
	}

	// Add last mesh found in the file (as there is no "o" keyword afterwards to trigger the adding code)
	Mesh loadedMesh(finalVerts, cooking, finalIndices, meshType);
	mGeometry.push_back(loadedMesh);

	// Remove all data from vectors
	verts.clear();
	normals.clear();
	uvs.clear();
	indices.clear();
	return true;
}

bool ModelImporter::Read(physx::PxCooking* cooking)
{
	return _ReadOBJ(cooking);
}

Mesh ModelImporter::GetSubMesh(int idx)
{
	return mGeometry[idx];
}

std::vector<Mesh> Pinball::ModelImporter::GetGeometry()
{
	return mGeometry;
}

bool ModelImporter::Close()
{
	// close file handles
	mObjFile->close();
	mMtlFile->close();

	// delete pointers
	delete mObjFile;
	delete mMtlFile;

	// reinit pointers
	mObjFile = nullptr;
	mMtlFile = nullptr;

	return true;
}

ModelImporter::~ModelImporter()
{
	// Makes sure file handles are closed
	if (mObjFile != nullptr && mMtlFile != nullptr)
		Close();
}
