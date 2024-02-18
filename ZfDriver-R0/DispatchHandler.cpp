#include "DispatchHandler.h"
#include "IoctlUtils.h"
#include "Utils.h"
#include "KM.h"

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
	DbgPrint("[ZfDriver] Write");
	IOCTL_TRANS_WRITE* trans = (IOCTL_TRANS_WRITE*)hContext->pIoBuffer;
	if (Utils::MDLWriteMemory(trans->pid, trans->address, trans->size, &(trans->data[0])))
	{
		return STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("[ZfDriver] Write Error! Pid:%d, Address:%lx, Size:%d ", trans->pid, trans->address, trans->size);
		return STATUS_UNSUCCESSFUL;
	}
}

NTSTATUS DispatchHandler::ForceDeleteFile(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] Force Delete File");
	WCHAR filePathBuf[MAX_PATH] = { 0 };
	wcscat(filePathBuf, L"\\??\\");
	wcscat(filePathBuf, (PCWCHAR)hContext->pIoBuffer);
	UNICODE_STRING filePath;
	RtlInitUnicodeString(&filePath, filePathBuf);
	if (Utils::ForceDeleteFile(filePath))
	{
		return STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("[ZfDriver] Force Delete File Error! FilePath: %ls", filePathBuf);
		return STATUS_UNSUCCESSFUL;
	}
}

NTSTATUS DispatchHandler::GetModuleBase(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] Get Module Base");
	IOCTL_TRANS_GET_MODULE_BASE* trans = (IOCTL_TRANS_GET_MODULE_BASE*)hContext->pIoBuffer;
	do
	{
		PEPROCESS pEProcess = NULL;
		PsLookupProcessByProcessId((HANDLE)trans->pid, &pEProcess);
		if (pEProcess == NULL)
			break;

		DWORD64 base = 0;
		UNICODE_STRING moduleName;
		RtlInitUnicodeString(&moduleName, (PWCHAR)trans->moduleName);
		base = Utils::GetModuleBaseWow64(pEProcess, moduleName);
		if (base != 0)
		{
			*(PDWORD64)(hContext->pIoBuffer) = base;
			return STATUS_SUCCESS;
		}
		base = Utils::GetModuleBase64(pEProcess, moduleName);
		if (base != 0)
		{
			*(PDWORD64)(hContext->pIoBuffer) = base;
			return STATUS_SUCCESS;
		}
	} while (false);
	DbgPrint("[ZfDriver] Get Module Base Error! Pid:%d, ModuleName:%ls ", trans->pid, trans->moduleName);
	return STATUS_UNSUCCESSFUL;
}


NTSTATUS DispatchHandler::KBD(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] KBD");
	IOCTL_TRANS_KBD* pTransStart = (IOCTL_TRANS_KBD*)hContext->pIoBuffer;
	IOCTL_TRANS_KBD* pTransEnd = pTransStart + 1;
	ULONG inputDataConsumed;
	KM::gKoMCallBack.KeyboardClassServiceCallback(KM::gKoMCallBack.KdbDeviceObject,
		(KM::PKEYBOARD_INPUT_DATA)pTransStart,
		(KM::PKEYBOARD_INPUT_DATA)pTransEnd,
		&inputDataConsumed);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchHandler::MOU(PHandlerContext hContext)
{
	DbgPrint("[ZfDriver] MOU");
	IOCTL_TRANS_MOU* pTransStart = (IOCTL_TRANS_MOU*)hContext->pIoBuffer;
	IOCTL_TRANS_MOU* pTransEnd = pTransStart + 1;
	ULONG inputDataConsumed;
	KM::gKoMCallBack.MouseClassServiceCallback(KM::gKoMCallBack.KdbDeviceObject,
		(KM::PMOUSE_INPUT_DATA)pTransStart,
		(KM::PMOUSE_INPUT_DATA)pTransEnd,
		&inputDataConsumed);
	return STATUS_SUCCESS;
}