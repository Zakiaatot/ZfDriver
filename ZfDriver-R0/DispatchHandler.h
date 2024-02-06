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
	NTSTATUS ForceDelete(PHandlerContext hContext);
}

#endif // !_DISPATCH_HANDLER_H_


