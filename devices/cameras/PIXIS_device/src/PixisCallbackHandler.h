#ifndef PIXISCAMERALISTENER_H
#define PIXISCAMERALISTENER_H

#include "picam.h"

#include <map>


class PixisCameraListener
{
public:
	virtual PicamError PIL_CALL AcquisitionUpdated(PicamHandle device, const PicamAvailableData* available, const PicamAcquisitionStatus* status) = 0;
};

class PixisCallbackHandler
{
public:
	
	PicamError addPixisCallbackListener(const PicamHandle& cameraHandle, PixisCameraListener* listener);

	//void addPixisCallbackListener(const PicamHandle& cameraHandle, PixisCameraListener* listener) {
	//	listeners.emplace(cameraHandle, listener);
	//}

	static PicamError PIL_CALL AcquisitionUpdated(PicamHandle device, const PicamAvailableData* available, const PicamAcquisitionStatus* status);

	//static PicamError PIL_CALL AcquisitionUpdated(PicamHandle device, const PicamAvailableData* available, const PicamAcquisitionStatus* status) {
	//	auto it = listeners.find(device);

	//	if (it != listeners.end()) {
	//		//found
	//		return it->second->AcquisitionUpdated(device, available, status);
	//	}
	//	else {
	//		return PicamError_None;
	//	}

	//	//		for (auto listener : listeners) {
	//	//			listener.second->AcquisitionUpdated(device, available, status);
	//	//		}

	//}

	static std::map<PicamHandle, PixisCameraListener*> listeners;
};


#endif

