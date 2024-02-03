#include <windef.h>
#include "DispatchHandler.h"

// For Test
// IN: DWORD num  OUT: DWORD num+1
NTSTATUS DispatchHandler::Test(PHandlerContext hContext)
{
	DWORD num = 0;
	memcpy(&num, hContext->pIoBuffer, sizeof(DWORD));
	num++;
	memcpy(hContext->pIoBuffer, &num, sizeof(DWORD));
	return STATUS_SUCCESS;
}

NTSTATUS DispatchHandler::Read(PHandlerContext hContext)
{
	return STATUS_SUCCESS;
}

NTSTATUS DispatchHandler::Write(PHandlerContext hContext)
{
	return STATUS_SUCCESS;
}

NTSTATUS DispatchHandler::ForceDelete(PHandlerContext hContext)
{
	return STATUS_SUCCESS;
}