#ifndef BLACKFLYEVENT_H
#define BLACKFLYEVENT_H

#include "STI_Device.h"
#include "Image.h"
#include "Spinnaker.h"

#include <thread>
#include <condition_variable>

class BlackflyDevice;

class BlackflyInitializeEvent : public STI_Device::SynchronousEventAdapter
{
public:
	BlackflyInitializeEvent(double time, BlackflyDevice* cameraDevice, int totalImages);

	void loadEvent();
	void playEvent();

	int totalImages;

private:
	BlackflyDevice* cameraDevice;
};

class BlackflyEvent : public STI_Device::SynchronousEventAdapter
{
public:

	enum BlackflyEventMode { Normal, Mean, Photodetector };

	BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image);
	BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, BlackflyEventMode mode);
	BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, const shared_ptr<Image>& imageBuffer, BlackflyEventMode mode);

	//			: SynchronousEvent(time, cameraDevice_), cameraDevice(cameraDevice_),
	//			image(image), imageWriter(imageWriter)
	//		{
	//			image->imageData.reserve(image->getImageSize());	//reserve memory for image data
	//		}

	//		void reset();	//override
	void stop();	//override

	void setupEvent() { }
	void loadEvent();
	void playEvent();
	void collectMeasurementData();

	void waitBeforeCollectData();

	BlackflyDevice* cameraDevice;

	Spinnaker::ImagePtr pResultImage;
	Spinnaker::ImagePtr convertedImage;

	shared_ptr<Image> image;
	shared_ptr<Image> imageBuffer;
	int imageCount;		//for keeping track of the mean

						//		AbsorptionImageType absType;

	BlackflyEventMode mode;

private:
	Int64 getTotal(const vector<IMAGEWORD>& data);

	void setAcquisitionRunning(bool acquiring);

	std::mutex acquisitionMutex;
	std::condition_variable acquisitionCondition;
	bool acquisitionRunning;

};



#endif

