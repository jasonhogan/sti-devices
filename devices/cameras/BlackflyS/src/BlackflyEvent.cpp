
#include "BlackflyEvent.h"
#include "BlackflyDevice.h"



BlackflyInitializeEvent::BlackflyInitializeEvent(double time, BlackflyDevice* cameraDevice, int totalImages)
: STI_Device::SynchronousEventAdapter(time, cameraDevice), cameraDevice(cameraDevice), totalImages(totalImages) 
{
}

void BlackflyInitializeEvent::loadEvent()
{
//	cameraDevice->camera->ClearImageBuffer();
//	cameraDevice->camera->get

}


void BlackflyInitializeEvent::playEvent()
{

//	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);	//unlock
//	cameraDevice->camera->SetIntegerNodeValue("AcquisitionFrameCount", totalImages);
	
	cameraDevice->setStreamNodeValue("StreamBufferCountManual", totalImages);

	cameraDevice->setNodeValue("TriggerMode", "On");

	//	cout << "Parsed::totalImages = " << totalImages << endl;

	//cameraDevice->camera->SetImageBufferFrameCount(totalImages);

	//UINT32 pendingImages;
	//INT64 frameCount;

	//cameraDevice->camera->GetIntegerNodeValue("AcquisitionFrameCount", frameCount);

	//pendingImages = cameraDevice->camera->GetPendingImagesCount();
	//cout << "Parsed::Pending: " << pendingImages << endl;
	//cout << "Parsed::frameCount: " << frameCount << endl;
}


BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, const shared_ptr<Image>& imageBuffer, BlackflyEventMode mode)
	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), imageBuffer(imageBuffer), mode(mode)
{
	imageCount = 0;
}

BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, BlackflyEventMode mode)

	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), mode(mode)
{
	image->imageData.reserve(image->getImageSize());	//reserve memory for image data
	imageCount = 0;
}

BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image)
	: BlackflyEvent::BlackflyEvent(time, cameraDevice_, image, Normal)
{
}

void BlackflyEvent::playEvent()
{
	//set exposure time for this image
	cameraDevice->exposureTimeNodeValue->setValue(STI::Utils::valueToString(image->exposureTime));

	bool status;
	// set trigger selector to frame start
	//status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
	//status = cameraDevice->camera->SetStringNodeValue("TriggerSelector", "FrameStart");
	status = cameraDevice->setNodeValue("TriggerSelector", "FrameStart");

	//status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 1);
	//status = cameraDevice->camera->CommandNodeExecute("AcquisitionStart");
	//cameraDevice->camera->AcquisitionStart();
	cameraDevice->camera->BeginAcquisition();

	//Trigger setup
	if (cameraDevice->isHardwareTriggered()) {
		//wait for hardware trigger
//		status = cameraDevice->camera->CommandNodeExecute("Line1");		//hardware trigger
	}
	else {
		//send software trigger NOW
		//status = cameraDevice->camera->CommandNodeExecute("TriggerSoftware");	//software trigger
		cameraDevice->camera->TriggerSoftware.Execute();
	}
}


void BlackflyEvent::stop()
{
	//Abort
//	bool status;
//	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStop");
	cameraDevice->camera->AcquisitionAbort();
	cameraDevice->setNodeValue("TriggerMode", "Off");
}

void BlackflyEvent::waitBeforeCollectData()
{
	//bool success = false;

	//while (!success && cameraDevice->running()) {
	//	success = cameraDevice->camera->WaitForImage(1);	//1 second timeout
	//														//		cout << "WaitForImage: " << getEventNumber()  << " pending: " << cameraDevice->camera->GetPendingImagesCount() << endl;
	//	
	//}
	pResultImage = cameraDevice->camera->GetNextImage();	//blocks
}


void BlackflyEvent::collectMeasurementData()
{
	//Pulls the image from the camera and stores it in an Image
//	PixelFormat_Mono8,HQ_LINEAR
	convertedImage = pResultImage->Convert(Spinnaker::PixelFormat_Mono12p, Spinnaker::NO_COLOR_PROCESSING);
	pResultImage->Release();	//removes image from camera buffer

	

//	smcs::IImageInfo imageInfo = nullptr;
	//cameraDevice->camera->GetImageInfo(&imageInfo);

	//if (cameraDevice->camera->GetPendingImagesCount() < 1 || imageInfo == 0) {
	//	//error; image not in buffer
	//	image->imageData.clear();	//ImageWriter will not include this image
	//	return;
	//}

	//imageInfo is not null

	image->clearMetaData();	//reset;  needed in this Image was used on previous shot
	image->addMetaData("PaneTag", image->paneTag);
	image->addMetaData("Downsample", STI::Utils::valueToString(image->downsample));
	image->addMetaData("ExposureTime", STI::Utils::valueToString(image->exposureTime));
	image->addMetaData("Gain", STI::Utils::valueToString(image->gain));
//	image->addMetaData("CameraTimestamp", STI::Utils::valueToString(imageInfo->GetCameraTimestamp()));

//	UINT32 sizeX, sizeY;
//	imageInfo->GetSize(sizeX, sizeY);
//
//	image->setImageHeight(sizeY);
//	image->setImageWidth(sizeX);
//
//	int linesize = imageInfo->GetLineSize();
//	int rawdatasize = imageInfo->GetRawDataSize();
//
////	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
//
//	image->imageData.clear();	//important if this Image is reused on multiple shots
//
//								//copy image from camera buffer, line by line
//	UINT8* lineData;
//	std::vector<IMAGEWORD> unpackedLine(sizeX, 0);
//	for (unsigned int j = 0; j < sizeY; j++) {	// j is image line number
//												//Get next line and insert it into vector
//		lineData = imageInfo->GetRawData(j);
//		cameraDevice->unpackLine(lineData, unpackedLine);	//convert Mono10Packed format into shorts (16 bits per pixel)
//		image->imageData.insert(image->imageData.end(), unpackedLine.begin(), unpackedLine.end());
//	}
//
//	cameraDevice->camera->PopImage(imageInfo);	//removes image from camera buffer

	if (mode == Mean) {
		double res;

		if (imageBuffer->imageData.size() < image->imageData.size()) {
			imageBuffer->imageData.assign(image->imageData.size(), 0);
		}

		for (unsigned p = 0; p < image->imageData.size(); ++p) {
			res = static_cast<double>(imageBuffer->imageData.at(p))*(imageCount - 1)
				+ static_cast<double>(image->imageData.at(p));

			imageBuffer->imageData.at(p) = static_cast<IMAGEWORD>(res / imageCount);
		}
	}

	//Max sum is (2^10)*1936*1216 = 2,410,676,224.  This requires 32 bits.
	if (mode == Photodetector && eventMeasurements.size() == 1)
	{
		Int64 total = getTotal(image->imageData);
		double result = static_cast<double>(total);		//MixedValue doesn't support Int64 or unsigned long
		eventMeasurements.at(0)->setData(result);
	}
}

Int64 BlackflyEvent::getTotal(const vector<IMAGEWORD>& data)
{
	Int64 total = 0;
	for (auto& d : data) {
		total += d;
	}
	return total;
}

