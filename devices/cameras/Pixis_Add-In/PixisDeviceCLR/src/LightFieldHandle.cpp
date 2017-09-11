
#include "LightFieldHandle.h"

LightFieldHandle::LightFieldHandle()
{
}

void LightFieldHandle::aquire()
{
	if (aquireCB != 0) {
		aquireCB();
	}
}

void LightFieldHandle::stop()
{
	if (stopCB != 0) {
		stopCB();
//		goCB();
	}
}

void LightFieldHandle::go()
{
	if (goCB != 0) {
		goCB();
	}
}