
#include "Camera.h"

//#include <memory>
//using std::shared_ptr;

Camera::Camera()
{
	hbin = 1;
	vbin = 1;

	coolerSetpoint = 0;
	cameraTemperature = 0;

	imageWidth = 1;
	imageHeight = 1;

	minTemp = 0;
	maxTemp = 0;
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
	if (size >= 1 && size <= imageWidth) {

		hbin = getNearestWholeDivisorBelow(imageWidth, size);
		return true;
	}
	return false;
}

bool Camera::setVBin(int size)
{
	if (size >= 1 && size <= imageHeight) {
		vbin = getNearestWholeDivisorBelow(imageHeight, size);
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

