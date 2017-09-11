#ifndef CALLBACKS_H
#define CALLBACKS_H

typedef int(__stdcall *TESTCB)(int);
typedef void(__stdcall *AQUIRECB)(void);
typedef void(__stdcall *STOPCB)(void);

namespace Callback
{

struct Go
{
	typedef void(__stdcall *Func)(void);
	Go(Func cb) : cb(cb) {}
	Func cb;
};

};

//struct GOCB
//{
//	typedef void(__stdcall *CB)(void);
//	GOCB(CB cb) : cb(cb) {}
//	CB cb;
//};

//template<typename CB>
//struct Callback
//{
//	using Callback<CB>::Callback;
//	Callback(CB cb) : cb(cb) {}
//	CB cb;
//};
//
//typedef void(__stdcall *GO2CB)(void);
//struct GO2 : public Callback<GO2CB> 
//{
////	using Callback<GO2CB>::Callback;
//};


#endif
