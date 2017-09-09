#ifndef LIGHTFIELDHANDLE_H
#define LIGHTFIELDHANDLE_H

#include "callbacks.h"

class LightFieldHandle
{
public:
	LightFieldHandle();

	void aquire();

	//Install callbacks
	void install_AQUIRECB(AQUIRECB cb) { aquireCB = cb; }

private:
	//Callbacks
	AQUIRECB aquireCB;
};

#endif
