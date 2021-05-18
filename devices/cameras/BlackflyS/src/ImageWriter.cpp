
#include "ImageWriter.h"

#include <Magick++.h>
#include <string>
#include <sstream>

#include <iostream>

using std::string;

#include <filesystem>
namespace fs = std::experimental::filesystem;

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

//	return;

	for (auto image : images) {
		
		if (image->getImageDataSize() == 0) {
			break;
		}

//		image->spinImage->Save("C:\\Users\\Jason\\Code\\dev\\sti-devices\\devices\\cameras\\BlackflyS\\test2.tif");


		//works, but only in release build
		magickImage.read(image->getImageWidth(), image->getImageHeight(),
			"I", MagickCore::ShortPixel, image->getImageData());	//is this a deep copy of the image data?  Could be slow.  JMH
		
//		double size_x = image->getImageWidth();
//		double size_y = image->getImageHeight();

		if (image->downsample > 1) {
			double zoom_out_factor = 1.0 / static_cast<double>(image->downsample);
			magickImage.zoom(Magick::Geometry(image->getImageWidth()*zoom_out_factor, image->getImageHeight()*zoom_out_factor));
		}

		//magickImage.rotate(image->rotationAngle);
		setMetadata(magickImage, image);

		magickImages.push_back(magickImage);

		////Testing 'zoom' downsample method
		//Magick::Image magickImageTest;
		//magickImageTest.read(image->getImageWidth(), image->getImageHeight(),
		//	"I", MagickCore::ShortPixel, image->getImageData());	//is this a deep copy of the image data?  Could be slow.  JMH
		//magickImages.push_back(magickImageTest);
	}

	//Check that the target directory for the image exists; if not create it.
	fs::path imagePath(filename);
	if (!fs::exists(imagePath.parent_path())) {
		fs::create_directories(imagePath.parent_path());
	}

	//write the list of images for multipane tifs
	Magick::writeImages(magickImages.begin(), magickImages.end(), filename, true);
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

