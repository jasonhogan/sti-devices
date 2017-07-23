
#include "Image.h"

#include <sstream>
#include <math.h>

Image::Image(unsigned height, unsigned width)
	: imageHeight(height), imageWidth(width)
{
	resizeImage();
	filename = "default";
	extension = "tif";
}

Image::Image(const std::vector<int>& crop, int vbin, int hbin)
	: cropVector(crop), vbin(vbin), hbin(hbin)
{
	int hstart = cropVector.at(0);
	int hend = cropVector.at(1);
	int vstart = cropVector.at(2);
	int vend = cropVector.at(3);
	
	int goalWidth = hend - hstart + 1;
	int goalHeight = vend - vstart + 1;

	int superPixelsH = static_cast<int>( std::floor(1.0 * goalWidth / hbin) );
	int superPixelsV = static_cast<int>( std::floor(1.0 * goalHeight / vbin) );

	imageWidth = hbin*superPixelsH;
	imageHeight = vbin*superPixelsV;

	hend = hstart - 1 + imageWidth;
	vend = vstart - 1 + imageHeight;

	cropVector.at(1) = hend;
	cropVector.at(3) = vend;

	resizeImage();
}


void Image::addMetaData(const std::string& tag, const std::string& value)
{
	Metadatum meta(tag, value);
	metadata.push_back(meta);
}

int Image::getImageSize()
{
	return imageHeight*imageWidth;
}

std::string Image::getFilename()
{
	std::stringstream name;
	name << filename << "." << extension;

	return name.str();
}


void Image::setImageHeight(unsigned height)
{
	imageHeight = height;
	resizeImage();
}

void Image::setImageWidth(unsigned width)
{
	imageWidth = width;
	resizeImage();
}

void Image::resizeImage()
{
	imageData.resize(imageHeight*imageWidth);
}

