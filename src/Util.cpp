#pragma once

#include "Util.h"

bool strContains(std::string str, std::string substr)
{
	return str.find(substr) != std::string::npos;
}

std::string getFileContents(std::string path)
{
	std::ifstream fs(path);
	std::string ret = "";
	while (fs.good())
	{
		std::string currentLine = "";
		std::getline(fs, currentLine);
		ret += currentLine;
		if (!fs.eof())
			ret += "\n";
	}

	return ret;
}

float* mat4ToRaw(glm::mat4 mat)
{
	float* ret = new float[4 * 4];
	for (int i = 0; i < 4 * 4; i += 4)
	{
		// TODO: could just memcpy this
		ret[i] = mat[i / 4].x;
		ret[i + 1] = mat[i / 4].y;
		ret[i + 2] = mat[i / 4].z;
		ret[i + 3] = mat[i / 4].w;
	}

	return ret;
}