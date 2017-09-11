#ifndef LIGHTFIELDHANDLE_H
#define LIGHTFIELDHANDLE_H

#include "callbacks.h"

class LightFieldHandle
{
public:
	LightFieldHandle();

	void aquire();
	void stop();
	void go();

	//Install callbacks
//	void install_AQUIRECB(AQUIRECB cb) { aquireCB = cb; }
//	void install_STOPCB(STOPCB cb) { stopCB = cb; }

	void install_CB(AQUIRECB cb) { aquireCB = cb; }
	void install_CB2(STOPCB cb) { stopCB = cb; }

	
	void install_CB(Callback::Go cb) { goCB = cb.cb; }


private:
	//Callbacks
	AQUIRECB aquireCB;
	STOPCB stopCB;

	Callback::Go::Func goCB;
};

#endif
