#ifndef _DISPATCH_HANDLER_H_
#define _DISPATCH_HANDLER_H_

#include <ntifs.h>

typedef struct _HandlerContext
{
	PNTSTATUS pStatus;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;
}HandlerContext, * PHandlerContext;

namespace DispatchHandler
{

}

#endif // !_DISPATCH_HANDLER_H_


