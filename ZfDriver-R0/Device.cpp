#include "Device.h"

NTSTATUS Device::CreateObject(IN PDRIVER_OBJECT pDriObj)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING deviceName;
	UNICODE_STRING symLinkName;

	// CREATE DEVICE
	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	status = IoCreateDevice(pDriObj, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);

	// BUFFER I/O
	pDevObj->Flags |= DO_BUFFERED_IO;

	// SYMBOL LINK
	RtlInitUnicodeString(&symLinkName, SYM_LINK_NAME);
	status = IoCreateSymbolicLink(&symLinkName, &deviceName);
	return status;
}

VOID Device::DeleteObject(IN PDRIVER_OBJECT pDriObj)
{
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING symLinkName;
	pDevObj = pDriObj->DeviceObject;
	IoDeleteDevice(pDevObj);
	RtlInitUnicodeString(&symLinkName, SYM_LINK_NAME);
	IoDeleteSymbolicLink(&symLinkName);
}