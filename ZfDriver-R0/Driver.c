#include<ntifs.h>

VOID DriverUnload(IN PDRIVER_OBJECT pDriObj)
{
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriObj, IN PUNICODE_STRING registryPath)
{
	pDriObj->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}