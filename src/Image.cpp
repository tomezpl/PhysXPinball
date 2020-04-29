#define STB_IMAGE_IMPLEMENTATION

#include "Renderer.h"

using namespace Pinball;

Image::Image()
{
	mWidth = mHeight = 0;
	mSpectrum = 0;
	mTexture = 0;
	mData = nullptr;
}

Image::Image(std::string filePath)
{
	mWidth = mHeight = 0;
	mSpectrum = 0;
	mTexture = 0;
	mData = nullptr;

	Load(filePath);
}

void Image::Load(std::string filePath)
{
	mData = stbi_load(filePath.c_str(), &mWidth, &mHeight, &mSpectrum, 4);
}

unsigned char* Image::GetData() 
{
	return mData;
}

unsigned int& Image::GetTexture()
{
	return mTexture;
}

int Image::DataLength()
{
	return mWidth * mHeight * mSpectrum;
}

int Image::Width()
{
	return mWidth;
}

int Image::Height()
{
	return mHeight;
}