#include "Dispatch.h"
#include "IoctlCode.h"
#include "DispatchHandler.h"

VOID Dispatch::Register(IN PDRIVER_OBJECT pDriObj)
{
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriObj->MajorFunction[i] = DispatchDefault;
	}

	pDriObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
}

NTSTATUS DispatchDefault(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;

	// IRP Stack
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	// Control Code
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

	// I/O Buffer
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;

	// DeviceIoControl nInBufferSize
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;

	// DeviceIoControl nOutBufferSize
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	// Generate Context
	HandlerContext handlerContext = { &status,pIrpStack,uIoControlCode,pIoBuffer,uInSize,uOutSize };

	// Different Code Handler
	switch (uIoControlCode)
	{

	}

	// Judge Success Or Failed
	if (status == STATUS_SUCCESS)
	{
		pIrp->IoStatus.Information = uOutSize;
	}
	else
	{
		pIrp->IoStatus.Information = 0;
	}
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}