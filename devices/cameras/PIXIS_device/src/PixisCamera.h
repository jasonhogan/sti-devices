
#ifndef PIXISCAMERA_H
#define PIXISCAMERA_H

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <condition_variable> 


#include "Camera.h"
#include "Image.h"
#include "picam.h"



// Prototype for conversion functions
static std::wstring StringToWString(const std::string& s);
static std::string WStringToString(const std::wstring& s);
static std::wstring GetEnumString(PicamEnumeratedType type, piint value);
//static PicamError PIL_CALL AcquisitionUpdated(PicamHandle device, const PicamAvailableData* available, const PicamAcquisitionStatus* status);

class PixisCamera : public Camera
{
public:

	PixisCamera();
	~PixisCamera();

	void init();

	bool InitializeCamera();

	bool getTemperature_i(double& temperature);
	bool setTemperature_i(double setpoint);

	bool getImage(int imageIndex, const std::shared_ptr<Image>& image);

	bool checkError(PicamError errorValue, const std::string& message);

	void resetAcquisitionIndex();

	void StartAcquisition(int imageIndex);
	void StopAcquisition();
	void WaitForAcquisition(int imageIndex);
	
	bool checkParametersAreCommited();
	bool refreshROI();

	double getExposureTime();
	bool setExposureTime(double time);

private:

	PicamHandle camera;
	PicamCameraID id;

	bool acquiring;
	int acquisitionIndex;

	piint readoutstride;
	piint frameSize;    //size of frame (bytes)
	piint frameStride;  //stride to next frame (bytes)

	piflt exposureTime;

	mutable std::mutex aquire_mutex;
	std::condition_variable aquire_condition;

//	const PicamAvailableData* availableData;
	PicamAvailableData* availableData;

};

#endif
