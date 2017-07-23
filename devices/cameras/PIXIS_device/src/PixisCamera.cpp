
#include "PixisCamera.h"

#include "picam.h"
#include "picam_advanced.h"

#include <memory>
#include <string>
#include <algorithm>

#include <iostream>
using std::wcout;
using std::endl;

using std::shared_ptr;


PixisCamera::PixisCamera() : Camera()
{
	init();
}

PixisCamera::~PixisCamera()
{
	checkError(Picam_CloseCamera(camera), "Picam_CloseCamera");
	checkError(Picam_UninitializeLibrary(), "Picam_UninitializeLibrary");
}

void PixisCamera::init()
{
	availableData = new PicamAvailableData();
	readoutstride = 0;
	acquiring = false;
}

bool PixisCamera::InitializeCamera()
{
	bool success = false;

	pibln initialized = false;

	if (checkError(Picam_IsLibraryInitialized(&initialized), "Picam_IsLibraryInitialized")) {

		if (!initialized) {
			success = checkError(Picam_InitializeLibrary(), "Picam_InitializeLibrary");
		}
		else {
			success = true;		//library is already initialized
			return success;
		}
	}

	if (!success) return success;		//library initialization error


	// - open the first camera if any or create a demo camera
	const pichar* string;
//	PicamAvailableData data;
//	PicamAcquisitionErrorsMask errors;

	if (Picam_OpenFirstCamera(&camera) == PicamError_None) {
		Picam_GetCameraID(camera, &id);
	}
	else
	{
		Picam_ConnectDemoCamera(
			PicamModel_Pixis100F,
			"0008675309",
			&id);
		Picam_OpenCamera(&id, &camera);
		printf("No Camera Detected, Creating Demo Camera\n");
	}
	Picam_GetEnumerationString(PicamEnumeratedType_Model, id.model, &string);
	printf("%s", string);
	printf(" (SN:%s) [%s]\n", id.serial_number, id.sensor_name);
	Picam_DestroyString(string);
	
	pibln readoutStartedEnabled = false;
	pibln readoutDoneEnabled = false;

//	EnableReadoutStatusCallbacks(camera, readoutStartedEnabled, readoutDoneEnabled);
//	DisableReadoutStatusCallbacks(camera, readoutStartedEnabled, readoutDoneEnabled);
	
//	PicamError error = PicamAdvanced_RegisterForAcquisitionUpdated(
//		camera, AcquisitionUpdatedTest);
//	checkError(error,"PicamAdvanced_RegisterForAcquisitionUpdated");


	// - handle an acquiring camera
//	pibln running;
//	PicamError error = Picam_IsAcquisitionRunning(device_, &running);
//	if (error == PicamError_None && running)
//	{
	//error = Picam_StopAcquisition( device_ );
	//	WaitForSingleObject(
	//		acquisitionInactive_,
	//		10000) != WAIT_OBJECT_0;
	//	if (running)
	//		DisplayError(L"Failed to stop camera.", error);
	//}

	//binning:  PicamRoi, page 107, also see 112 for constraints


	//	error = PicamAdvanced_RegisterForAcquisitionUpdated((void*)this, AcquisitionUpdated);
	//	error = PicamAdvanced_RegisterForAcquisitionUpdated(camera, AcquisitionUpdated2);

	checkError(
		Picam_GetParameterIntegerValue(camera, PicamParameter_ReadoutStride, &readoutstride),
		"Picam_GetParameterIntegerValue");

	checkError(
		Picam_SetParameterIntegerValue(camera, PicamParameter_TriggerResponse, PicamTriggerResponse_ReadoutPerTrigger),
		"Picam_SetParameterIntegerValue");
	
	checkError(
	Picam_SetParameterIntegerValue(camera, PicamParameter_TriggerDetermination, PicamTriggerDetermination_PositivePolarity),
		"Picam_SetParameterIntegerValue");
	
	//commit the changes
	pibln committed;

	checkError(
		Picam_AreParametersCommitted(camera, &committed),
		"Picam_AreParametersCommitted");

	if (!committed)
	{
		const PicamParameter* failed_parameter_array = NULL;
		piint failed_parameter_count = 0;
		Picam_CommitParameters(camera, &failed_parameter_array, &failed_parameter_count);
		if (failed_parameter_count) {
			//There are failed parameters.  For now, just clear the memory.
			Picam_DestroyParameters(failed_parameter_array);
		}
	}


	return success;
}

bool PixisCamera::getTemperature_i(double& temperature)
{

	PicamError error =
		Picam_ReadParameterFloatingPointValue(
			camera,
			PicamParameter_SensorTemperatureReading,
			&temperature);
	
	if (!checkError(error, "getTemperature_i")) {
		return false;	//error getting temperature
	}

	return true;
}

bool PixisCamera::setTemperature_i(double setpoint)
{
	return false;
}

bool PixisCamera::getImage(int imageIndex, const shared_ptr<Image>& image)
{
//	piint readoutStride_ = 0;               // - stride to next readout (bytes)
	piint framesPerReadout_ = 1;            // - number of frames in a readout
	piint frameStride_ = 0;                 // - stride to next frame (bytes)
	piint frameSize_ = 0;                   // - size of frame (bytes)
	pi64s imageDataVersion_ = 0;            // - current version of image data
	std::vector<pi16u> imageData_;          // - data from last frame

	bool success = false;

	if (availableData) {
		std::cout << "available data" << endl;
	}

	if (availableData && availableData->readout_count) {
			
		pi64s lastReadoutOffset = readoutstride * (availableData->readout_count - 1);
		pi64s lastFrameOffset = frameStride_ * (framesPerReadout_ - 1);
		const pibyte* frame =
			static_cast<const pibyte*>(availableData->initial_readout) +
			lastReadoutOffset + lastFrameOffset;
//		std::memcpy(&imageData_[0], frame, frameSize_);

//		(image->imageData)[0];

		std::memcpy(image->imageData.data(), frame, frameSize_);

		success = true;

		++imageDataVersion_;
	}

	return success;
}

void PixisCamera::StartAcquisition()
{
	std::unique_lock<std::mutex> lock(aquire_mutex);

//	pi64s NUM_FRAMES = 1;
//	checkError(
//		Picam_SetParameterLargeIntegerValue( camera, PicamParameter_ReadoutCount, NUM_FRAMES ), "Picam_SetParameterLargeIntegerValue");
//	PicamError error = Picam_StartAcquisition(camera);
	
	PicamError error = PicamError_None;


	PicamAcquisitionErrorsMask errMask;
	printf("Waiting for external trigger\n");
	if (!Picam_Acquire(camera, 1, -1, availableData, &errMask))
	{
		printf("Received external trigger\n\n");
//		printf("Center Three Points:\tFrame # \n");
//		PrintData((pibyte*)data.initial_readout, 1, readoutstride);
	}


	if (checkError(error, "StartAcquisition")) {
		acquiring = true;
		aquire_condition.notify_one();
	}
}

void PixisCamera::WaitForAcquisition()
{
	std::unique_lock<std::mutex> lock(aquire_mutex);
	if (!acquiring) {
		aquire_condition.wait(lock);
	}

	return;


//	PicamAvailableData data;
	PicamAcquisitionStatus status;
	PicamError err;

	piint TIMEOUT = -1;		//infinite

	int i = 0;

	pibln bRunning = true;
	while (bRunning)
	{
		i++;
		err = Picam_WaitForAcquisitionUpdate(camera, TIMEOUT, availableData, &status);
//		err = Picam_WaitForAcquisitionUpdate(camera, TIMEOUT, &data, &status);
//		err = Picam_WaitForAcquisitionUpdate(camera, TIMEOUT, data, &status);
		if (err == PicamError_None)
		{
			bRunning = status.running;
			if (!bRunning) {
				std::cout << "not running" << endl;
			}
//			framecount += data.readout_count;
//			PrintData((pibyte*)data.initial_readout, data.readout_count, readoutstride);
		}
		else if (err == PicamError_TimeOutOccurred)
		{
			printf("Terminating prematurely!  Try increasing time out\n");
			Picam_StopAcquisition(camera);
		}
	}
	std::cout << "Loops: " << i << endl;
}



void PixisCamera::StopAcquisition()
{
//	std::unique_lock<std::mutex> lock(aquire_mutex);

	PicamError error = Picam_StopAcquisition(camera);

	checkError(error, "StopAcquisition");

	//if (!checkError(error, "StopAcquisition")) {
	//	//error occurred
	//}

}


bool PixisCamera::checkParametersAreCommited()
{
	bool success = true;
	pibln committed = 0;
	Picam_AreParametersCommitted(camera, &committed);

	if (!committed)
	{
		const PicamParameter* failed_parameter_array = NULL;
		piint failed_parameter_count = 0;
		Picam_CommitParameters(camera, &failed_parameter_array, &failed_parameter_count);
		if (failed_parameter_count)
		{
			success = false;
			Picam_DestroyParameters(failed_parameter_array);
		}
		else {
			success = true;
		}
	}
	return !committed;
}


bool PixisCamera::checkError(PicamError errorValue, const std::string& message)
{
	bool success = true;

	if (errorValue == PicamError_None)
		return success;

	std::wstring details = StringToWString(message);
	details += L" (" + GetEnumString(PicamEnumeratedType_Error, errorValue) + L")";

	switch (errorValue) {
	case PicamError_None:
		success = true;
		break;
	default:
		success = false;
		break;
	}

	wcout << "Error: " << details << endl;

	////From Rois.cpp example in Princeton sample code.
	///* Get the error string, the called function allocates the memory */
	//const pichar *errString;
	//Picam_GetEnumerationString(PicamEnumeratedType_Error,
	//	(piint)errorValue,
	//	&errString);
	///* Print the error */
	//printf("%s\n", errString);

	///* Since the called function allocated the memory it must free it */
	//Picam_DestroyString(errString);

	return success;
}

//From picam examples, Advanced.cpp
std::wstring GetEnumString(PicamEnumeratedType type, piint value)
{
	const pichar* string;
	if (Picam_GetEnumerationString(type, value, &string) == PicamError_None)
	{
		std::string s(string);
		Picam_DestroyString(string);

		std::wstring w(s.length(), L'\0');
		std::copy(s.begin(), s.end(), w.begin());
		return w;
	}
	return std::wstring();
}


std::wstring StringToWString(const std::string& s)
{
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}


std::string WStringToString(const std::wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}