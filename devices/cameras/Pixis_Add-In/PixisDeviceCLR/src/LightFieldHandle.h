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
	int test(int x);

	//Install callbacks
	void install_CB(Callback::Test cb) { testCB = cb.cb; }
	void install_CB(Callback::Aquire cb) { aquireCB = cb.cb; }
	void install_CB(Callback::Go cb) { goCB = cb.cb; }
	void install_CB(Callback::Stop cb) { stopCB = cb.cb; }

private:

	//Callbacks
	Callback::Test::Func testCB;
	Callback::Aquire::Func aquireCB;
	Callback::Go::Func goCB;
	Callback::Stop::Func stopCB;
};

#endif
