
#include "ImageWriter.h"

#include <Magick++.h>
#include <string>
#include <sstream>

#include <iostream>

using std::string;


ImageWriter::ImageWriter()
{
	reset();
}


void ImageWriter::reset() 
{
	images.clear();
}

void ImageWriter::addImage(const std::shared_ptr<Image>& image)
{
	images.push_back(image);
}

void ImageWriter::write(const std::string& filename)
{
	saveToMultiPageGrey(filename);		//not sure why this function is called this.  JMH
}

void ImageWriter::saveToMultiPageGrey(const std::string& filename)
{
//	std::cout << "saveToMultiPageGrey" << std::endl;

	Magick::Image magickImage;
	std::list<Magick::Image> magickImages;

	const std::string magickMap("I");	//Not sure what this does... JMH

	if (images.empty()) {
		return;
	}

	for (auto image : images) {
		
		if (image->imageData.size() == 0) {
			break;
		}

		//works, but only in release build
		magickImage.read(image->getImageWidth(), image->getImageHeight(),
			"I", MagickCore::ShortPixel, image->imageData.data());	//is this a deep copy of the image data?  Could be slow.  JMH
							
		//magickImage.rotate(image->rotationAngle);
		setMetadata(magickImage, image);

		magickImages.push_back(magickImage);
	}

	//write the list of images for multipane tifs
	Magick::writeImages(magickImages.begin(), magickImages.end(), filename, true);

	//Delete image data
	reset();
}

void ImageWriter::setMetadata(Magick::Image& magickImage, const std::shared_ptr<Image>& image)
{
	std::stringstream attributes;

	for (auto meta : image->metadata) {
		attributes << meta.tag << ":" << meta.value << "; ";
	}

	std::string attr_str = attributes.str();

	magickImage.attribute("EXIF:ImageDescription", attr_str);
	magickImage.comment(attr_str);
}

