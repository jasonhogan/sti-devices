
#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include <memory>

#include "Image.h"


class Camera
{
public:

	Camera();

	virtual ~Camera() {}

	virtual bool setHBin(int size);
	virtual bool setVBin(int size);

	virtual double getTemperature();
	virtual bool setTemperature(double setpoint);

	virtual bool InitializeCamera() = 0;

	virtual bool getTemperature_i(double& temperature) = 0;
	virtual bool setTemperature_i(double setpoint) = 0;

	virtual bool getImage(int imageIndex, const std::shared_ptr<Image>& image) = 0;

//	virtual bool checkError(int errorValue, const std::string& message) = 0;

	int hbin;
	int vbin;
	
	double coolerSetpoint;

protected:

	int imageWidth;
	int	imageHeight;

	double	minTemp;
	double	maxTemp;
	double cameraTemperature;

private:

	int getNearestWholeDivisorBelow(int dividend, int divisor);

};

#endif
