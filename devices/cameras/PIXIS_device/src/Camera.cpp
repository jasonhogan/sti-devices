
#include "Camera.h"

//#include <memory>
//using std::shared_ptr;

Camera::Camera()
{
	hbin = 1;
	vbin = 1;

	coolerSetpoint = 0;
	cameraTemperature = 0;

	maxImageH = 1;
	maxImageV = 1;

	minTemp = 0;
	maxTemp = 0;

	roi.X = 0;
	roi.Y = 0;
	roi.widthH = maxImageH;
	roi.widthV = maxImageV;
}

void Camera::getROI(std::vector<int>& cropVector)
{
	cropVector.clear();
	cropVector.push_back(roi.X);
	cropVector.push_back(roi.Y);
	cropVector.push_back(roi.widthH);
	cropVector.push_back(roi.widthV);
}


double Camera::getTemperature()
{
	double temp = -999;

	if (!getTemperature_i(temp)) {
		//error; report last know temperature
//		temp = cameraTemperature;
	}
	else {
		//Success.  Save temperature
		cameraTemperature = temp;
	}
	return cameraTemperature;
}

bool Camera::setTemperature(double setpoint)
{
	bool success = true;
	double oldSetpoint = coolerSetpoint;

	if (minTemp <= setpoint && setpoint <= maxTemp) {
		coolerSetpoint = setpoint;
	}
	else if (setpoint < minTemp) {
		coolerSetpoint = minTemp;
	}
	else if (setpoint > maxTemp) {
		coolerSetpoint = maxTemp;
	}
	else {
		success = false;
	}

	success = setTemperature_i(coolerSetpoint);

	if (!success) {
		coolerSetpoint = oldSetpoint;
	}

	return success;
}


bool Camera::setHBin(int size)
{
	if (size >= 1 && size <= roi.widthH) {

		hbin = getNearestWholeDivisorBelow(roi.widthH, size);
		return true;
	}
	return false;
}

bool Camera::setVBin(int size)
{
	if (size >= 1 && size <= roi.widthV) {
		vbin = getNearestWholeDivisorBelow(roi.widthV, size);
		return true;
	}
	return false;
}

int Camera::getNearestWholeDivisorBelow(int dividend, int divisor)
{
	int i = 1;

	for (i = divisor; i > 0; --i) {
		if (dividend % i == 0)
			break;
	}

	return i;
}

