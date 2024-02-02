#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <ntifs.h>

#define DEVICE_NAME L"\\Device\\ZfDriver"
#define SYM_LINK_NAME L"\\??\\ZfDriver"

namespace Device
{
	NTSTATUS CreateObject(IN PDRIVER_OBJECT pDriObj);
	VOID DeleteObject(IN PDRIVER_OBJECT pDriObj);
}

#endif // !_DEVICE_H_
