
#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>

#include "Spinnaker.h"

//typedef char WORD;
//typedef unsigned short      WORD;
//typedef unsigned char IMAGEWORD;
typedef unsigned short IMAGEWORD;

class Metadatum {
public:
	Metadatum(const std::string& tag, const std::string& value) : tag(tag), value(value) {}
	std::string tag;
	std::string value;
};

//template <class WORD>
class Image
{
public:

	Image(unsigned height, unsigned width);
	Image(const std::vector<int>& crop, int vbin, int hbin);

	void addMetaData(const std::string& tag, const std::string& value);
	void clearMetaData();

	int getImageSize();
	std::string getFilename();
	
	void setImageHeight(unsigned height);
	void setImageWidth(unsigned width);

	int getImageHeight() { return imageHeight; }
	int getImageWidth() { return imageWidth; }

	//std::vector<IMAGEWORD> imageData;
	Spinnaker::ImagePtr spinImage;
	void* getImageData();
	int getImageDataSize();

	std::vector<Metadatum> metadata;

	unsigned sizeofWORD;

	std::string filename;
	std::string extension;

	int vbin;
	int hbin;
	int downsample;
	std::string paneTag;
	double exposureTime;
	std::string gain;
	std::string gainConversion;

	std::vector<int> cropVector;

	double rotationAngle;	//rotation angle (counter-clockwise)

private:

	void init();

	void resizeImage();
	int imageHeight;
	int imageWidth;

};


#endif
