#include "Dispatch.h"
#include "IoctlUtils.h"
#include "DispatchHandler.h"

NTSTATUS DispatchDefault(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("[ZfDriver] Opened\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("[ZfDriver] Closed\n");
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
	HandlerContext handlerContext = { pIrpStack,uIoControlCode,pIoBuffer,uInSize,uOutSize };

	// Different Code Handler
	switch (uIoControlCode)
	{
	case IOCTL_CODE_TEST:
		status = DispatchHandler::Test(&handlerContext);
		break;
	case IOCTL_CODE_READ:
		status = DispatchHandler::Read(&handlerContext);
		break;
	case IOCTL_CODE_WRITE:
		status = DispatchHandler::Write(&handlerContext);
		break;
	case IOCTL_CODE_FORCE_DELETE_FILE:
		status = DispatchHandler::ForceDeleteFile(&handlerContext);
		break;
	case IOCTL_CODE_GET_MODULE_BASE:
		status = DispatchHandler::GetModuleBase(&handlerContext);
		break;
	case IOCTL_CODE_KBD:
		status = DispatchHandler::KBD(&handlerContext);
		break;
	case IOCTL_CODE_MOU:
		status = DispatchHandler::MOU(&handlerContext);
		break;
	case IOCTL_CODE_PROCESS_HIDE:
		status = DispatchHandler::ProcessHide(&handlerContext);
		break;
	case IOCTL_CODE_WINDOW_HIDE:
		status = DispatchHandler::WindowHide(&handlerContext);
		break;
	case IOCTL_CODE_GET_PROCESS_ID:
		status = DispatchHandler::GetProcessId(&handlerContext);
		break;
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