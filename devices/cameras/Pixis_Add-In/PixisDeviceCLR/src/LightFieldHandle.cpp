
#include "LightFieldHandle.h"
#include <string>



LightFieldHandle::LightFieldHandle()
{
}

void LightFieldHandle::aquire(int index)
{
	if (aquireCB != 0) {
		aquireCB(index);
	}
}

void LightFieldHandle::setSaveDir(std::string dir)
{

	if (setSaveDirCB != 0) {
		setSaveDirCB(dir);
	}
}

void LightFieldHandle::print(std::string message)
{

	if (printCB != 0) {
		printCB(message);
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
bool LightFieldHandle::externalTriggerOn()
{
	if (extTriggerCB != 0) {
		return extTriggerCB();
	}
	return false;
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
