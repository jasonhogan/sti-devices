
#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>

//typedef char WORD;
typedef unsigned short      WORD;

class Metadatum {
public:
	Metadatum(const std::string& tag, const std::string& value) : tag(tag), value(value) {}
	std::string tag;
	std::string value;
};

class Image
{
public:

	Image(unsigned height, unsigned width);
	Image(const std::vector<int>& crop, int vbin, int hbin);

	void addMetaData(const std::string& tag, const std::string& value);

	int getImageSize();
	std::string getFilename();
	
	void setImageHeight(unsigned height);
	void setImageWidth(unsigned width);

	int getImageHeight() { return imageHeight; }
	int getImageWidth() { return imageWidth; }

	std::vector<WORD> imageData;
	std::vector<Metadatum> metadata;

	unsigned sizeofWORD;

	std::string filename;
	std::string extension;

	int vbin;
	int hbin;

	std::vector<int> cropVector;

	double rotationAngle;	//rotation angle (counter-clockwise)

private:

	void init();

	void resizeImage();
	int imageHeight;
	int imageWidth;

};


#endif
