#ifndef _DISPATCH_HANDLER_H_
#define _DISPATCH_HANDLER_H_

#include <ntifs.h>
#include <windef.h>

typedef struct _HandlerContext
{
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;
}HandlerContext, * PHandlerContext;

namespace DispatchHandler
{
	NTSTATUS Test(PHandlerContext hContext);
	NTSTATUS Read(PHandlerContext hContext);
	NTSTATUS Write(PHandlerContext hContext);
	NTSTATUS ForceDeleteFile(PHandlerContext hContext);
	NTSTATUS GetModuleBase(PHandlerContext hContext);
	NTSTATUS KBD(PHandlerContext hContext);
	NTSTATUS MOU(PHandlerContext hContext);
	NTSTATUS ProcessHide(PHandlerContext hContext);
	NTSTATUS WindowHide(PHandlerContext hContext);
}

#endif // !_DISPATCH_HANDLER_H_


