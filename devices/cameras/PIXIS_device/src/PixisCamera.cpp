
#include "PixisCamera.h"

#include "picam.h"
#include "picam_advanced.h"

#include <memory>
#include <string>
//#include <algorithm>

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
	frameSize = 0;        
	frameStride = 0;
	exposureTime = 0;
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
		}
	}

	if (!success) return false;		//library initialization error


	// - open the first camera if any or create a demo camera
	const pichar* string;

	if ( checkError(Picam_OpenFirstCamera(&camera), "Picam_OpenFirstCamera") ) {
		checkError(Picam_GetCameraID(camera, &id), "Picam_GetCameraID");
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
	

	//binning:  PicamRoi, page 107, also see 112 for constraints

	success = true;

	success &= checkError(
		Picam_GetParameterIntegerValue(camera, PicamParameter_ReadoutStride, &readoutstride),
		"PicamParameter_ReadoutStride");

	// - cache frame size
	success &= checkError(
		Picam_GetParameterIntegerValue(camera, PicamParameter_FrameSize, &frameSize),
		"PicamParameter_FrameSize");

	success &= checkError(
		Picam_GetParameterIntegerValue(camera, PicamParameter_FrameStride, &frameStride),
		"PicamParameter_FrameStride");

	success &= checkError(
		Picam_SetParameterIntegerValue(camera, PicamParameter_TriggerResponse, PicamTriggerResponse_ReadoutPerTrigger),
		"Picam_SetParameterIntegerValue");
	
	success &= checkError(
		Picam_SetParameterIntegerValue(camera, PicamParameter_TriggerDetermination, PicamTriggerDetermination_PositivePolarity),
		"Picam_SetParameterIntegerValue");
	
	const PicamRoisConstraint  *constraint;
	success &= checkError(
		Picam_GetParameterRoisConstraint(camera,
			PicamParameter_Rois,
			PicamConstraintCategory_Required,
			&constraint), "Picam_GetParameterRoisConstraint");

	if (success) {
		maxImageH = (piint)constraint->width_constraint.maximum;
		maxImageV = (piint)constraint->height_constraint.maximum;

		success &= checkError(Picam_DestroyRoisConstraints(constraint), "Picam_DestroyRoisConstraints");

		refreshROI();
	}

	success &= checkParametersAreCommited();

	return success;
}


bool PixisCamera::refreshROI()
{
	bool success;
	const PicamRois	*region;		 /* Region of interest  */

	success = checkError(
		Picam_GetParameterRoisValue(camera,
		PicamParameter_Rois,
		&region),
		"Picam_GetParameterRoisValue");

	if (success && region != 0 && region->roi_count > 0) {
		roi.X = region->roi_array[0].x;
		roi.Y = region->roi_array[0].y;
		roi.widthH = region->roi_array[0].width;
		roi.widthV = region->roi_array[0].height;
		
		success =  setHBin(region->roi_array[0].x_binning);
		success &= setVBin(region->roi_array[0].y_binning);
	}

	return success;
}

double PixisCamera::getExposureTime()
{
	piflt exposure;
	PicamError error =
		Picam_GetParameterFloatingPointValue(
			camera,
			PicamParameter_ExposureTime,
			&exposure);

	if (checkError(error, "Picam_GetParameterFloatingPointValue")) {
		exposureTime = exposure;
	}

	return exposureTime;
}

bool PixisCamera::setExposureTime(double time)
{
	PicamError error =
		Picam_SetParameterFloatingPointValueOnline(
			camera,
			PicamParameter_ExposureTime,
			time);

	if (checkError(error, "Picam_SetParameterFloatingPointValueOnline")) {
		exposureTime = time;
		return true;
	}
	return false;
}


bool PixisCamera::getTemperature_i(double& temperature)
{

	PicamError error =
		Picam_ReadParameterFloatingPointValue(
			camera,
			PicamParameter_SensorTemperatureReading,
			&temperature);
	
	if (!checkError(error, "Picam_ReadParameterFloatingPointValue")) {
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
	piint framesPerReadout_ = 1;            // - number of frames in a readout

	bool success = false;

	if (availableData && availableData->readout_count) {
			
		pi64s lastReadoutOffset = readoutstride * (availableData->readout_count - 1);		//should use imageIndex
		pi64s lastFrameOffset = frameStride * (framesPerReadout_ - 1);	//==0
		const pibyte* frame =
			static_cast<const pibyte*>(availableData->initial_readout) +
			lastReadoutOffset + lastFrameOffset;

		image->imageData.resize(frameSize / image->sizeofWORD);

		std::memcpy(image->imageData.data(), frame, frameSize);

		success = true;
	}

	return success;
}

void PixisCamera::StartAcquisition(int imageIndex)
{
	std::unique_lock<std::mutex> lock(aquire_mutex);

	piint readout_timeout = -1;  //infinite
	pi64s readout_count = 1;

	PicamAcquisitionErrorsMask errMask;
	printf("Waiting for external trigger\n");
	if (!Picam_Acquire(camera, readout_count, readout_timeout, availableData, &errMask))
	{
		acquisitionIndex = imageIndex;
		printf("Received external trigger\n\n");
//		printf("Center Three Points:\tFrame # \n");
//		PrintData((pibyte*)data.initial_readout, 1, readoutstride);
	}

	PicamError error = PicamError_None;

	if (checkError(error, "StartAcquisition")) {
		acquiring = true;
		aquire_condition.notify_one();
	}
}

void PixisCamera::WaitForAcquisition(int imageIndex)
{
	std::unique_lock<std::mutex> lock(aquire_mutex);
	if (acquiring) {
		aquire_condition.wait(lock);
	}
}


void PixisCamera::StopAcquisition()
{
	bool success = checkError(
		Picam_StopAcquisition(camera), "Picam_StopAcquisition");

	if (success) {
		std::unique_lock<std::mutex> lock(aquire_mutex);
		acquiring = false;
	}
}

void PixisCamera::resetAcquisitionIndex()
{
	acquisitionIndex = -1;
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