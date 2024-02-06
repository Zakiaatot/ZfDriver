#include "DispatchHandler.h"
#include "IoctlUtils.h"
#include "Utils.h"

// For Test
// IN: DWORD num  OUT: DWORD num+1
NTSTATUS DispatchHandler::Test(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] Test");
	DWORD num = 0;
	memcpy(&num, hContext->pIoBuffer, sizeof(DWORD));
	num++;
	memcpy(hContext->pIoBuffer, &num, sizeof(DWORD));
	return STATUS_SUCCESS;
}

// Read Memory
// IN: IOCTL_TRANS_READ (pid,address,size)  OUT: BYTE[] data
NTSTATUS DispatchHandler::Read(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] Read");
	IOCTL_TRANS_READ* trans = (IOCTL_TRANS_READ*)hContext->pIoBuffer;
	if (Utils::MDLReadMemory(trans->pid, trans->address, trans->size, (BYTE*)hContext->pIoBuffer))
	{
		return STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("[ZfDriver] Read Error! Pid:%d, Address:%lx, Size:%d ", trans->pid, trans->address, trans->size);
		return STATUS_UNSUCCESSFUL;
	}
}

NTSTATUS DispatchHandler::Write(PHandlerContext hContext)
{
	return STATUS_SUCCESS;
}

NTSTATUS DispatchHandler::ForceDelete(PHandlerContext hContext)
{
	return STATUS_SUCCESS;
}