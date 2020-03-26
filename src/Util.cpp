#pragma once

#include "Util.h"

bool strContains(std::string str, std::string substr)
{
	return str.find(substr) != std::string::npos;
}