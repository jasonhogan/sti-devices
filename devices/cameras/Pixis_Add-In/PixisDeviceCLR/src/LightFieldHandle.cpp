
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