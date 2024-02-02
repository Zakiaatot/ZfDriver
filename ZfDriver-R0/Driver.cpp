#include <ntifs.h>
#include <windef.h>
#include "Device.h"
#include "Dispatch.h"


VOID DriverUnload(IN PDRIVER_OBJECT pDriObj)
{
	Device::DeleteObject(pDriObj);
	DbgPrint("ZfDriver Unloaded.\n");
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriObj, IN PUNICODE_STRING registryPath)
{
	Device::CreateObject(pDriObj);
	Dispatch::Register(pDriObj);
	pDriObj->DriverUnload = DriverUnload;
	DbgPrint("ZfDriver Loaded.\n");
	return STATUS_SUCCESS;
}