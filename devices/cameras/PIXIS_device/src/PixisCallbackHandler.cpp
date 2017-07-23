
#include "PixisCallbackHandler.h"
#include "picam.h"
#include "picam_advanced.h"

#include <map>


std::map<PicamHandle, PixisCameraListener*> PixisCallbackHandler::listeners;


PicamError PixisCallbackHandler::addPixisCallbackListener(const PicamHandle& cameraHandle, PixisCameraListener* listener) {
	listeners.emplace(cameraHandle, listener);

	return PicamAdvanced_RegisterForAcquisitionUpdated(cameraHandle, PixisCallbackHandler::AcquisitionUpdated);

}

PicamError PIL_CALL PixisCallbackHandler::AcquisitionUpdated(PicamHandle device, const PicamAvailableData* available, const PicamAcquisitionStatus* status) {
	auto it = listeners.find(device);

	if (it != listeners.end()) {
		//found
		return it->second->AcquisitionUpdated(device, available, status);
	}
	else {
		return PicamError_InvalidHandle;
	}
}
