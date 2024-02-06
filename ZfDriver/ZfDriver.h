#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

#include "Windows.h"

class ZfDriver
{
public:
	static BOOL Install();
	static VOID Uninstall();
	static DWORD Test(DWORD num);
};

#endif // !_ZFDRIVER_H_
