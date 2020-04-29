#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <glm/mat4x4.hpp>

// Utility: returns true if str contains substr
bool strContains(std::string str, std::string substr);

// Utility: returns contents of a file as text. Used to get shader sources.
std::string getFileContents(std::string path);

// Utility: Converts a glm mat4 type into a raw 4x4 float array.
float* mat4ToRaw(glm::mat4 mat);