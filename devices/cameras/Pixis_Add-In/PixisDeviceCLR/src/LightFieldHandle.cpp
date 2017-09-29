
#include "LightFieldHandle.h"

LightFieldHandle::LightFieldHandle()
{
}

void LightFieldHandle::aquire(int index)
{
	if (aquireCB != 0) {
		aquireCB(index);
	}
}

void LightFieldHandle::stop()
{
	if (stopCB != 0) {
		stopCB();
	}
}
int LightFieldHandle::test(int x)
{
	if (testCB != 0) {
		return testCB(x);
	}
	return 0;
}

void LightFieldHandle::go()
{
	if (goCB != 0) {
		goCB();
	}
}

bool LightFieldHandle::isReadyToAquire()
{
	if (isReadyCB != 0) {
		return isReadyCB();
	}
	return false;
}
void LightFieldHandle::clearImageCount()
{
	if (clearCountCB != 0) {
		clearCountCB();
	}
}
void LightFieldHandle::incrementImageCount()
{
	if (incrementCountCB != 0) {
		incrementCountCB();
	}
}
