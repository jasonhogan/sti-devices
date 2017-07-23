
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

	void StartAcquisition();
	void StopAcquisition();
	void WaitForAcquisition();
	
	bool PixisCamera::checkParametersAreCommited();

private:

	PicamHandle camera;
	PicamCameraID id;

	bool acquiring;

	piint readoutstride;

	mutable std::mutex aquire_mutex;
	std::condition_variable aquire_condition;

//	const PicamAvailableData* availableData;
	PicamAvailableData* availableData;

};

#endif
