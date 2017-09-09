
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
