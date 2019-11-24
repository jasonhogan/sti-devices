

#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include "Image.h"
#include <Magick++.h>
#include <memory>

class ImageWriter
{
public:

	ImageWriter();

	void reset();
	
	void addImage(const std::shared_ptr<Image>& image);
	
	void write(const std::string& filename);


private:

	void setMetadata(Magick::Image& magickImage, const std::shared_ptr<Image>& image);

	void saveToMultiPageGrey(const std::string& filename);

	std::vector<std::shared_ptr<Image>> images;

};


#endif
