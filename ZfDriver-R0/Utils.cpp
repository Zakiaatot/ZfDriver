#include <ntifs.h>
#include "Utils.h"


BOOL Utils::MDLReadMemory(IN DWORD pid, IN PVOID address, IN DWORD size, OUT BYTE* res)
{
	BOOL bRet = TRUE;
	PEPROCESS process = NULL;
	PsLookupProcessByProcessId((HANDLE)pid, &process);
	if (process == NULL)
	{
		return FALSE;
	}
	BYTE* getData;
	__try
	{
		getData = (BYTE*)ExAllocatePool(PagedPool, size);
	}
	__except (1)
	{
		return FALSE;
	}
	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);
	__try
	{
		ProbeForRead(address, size, 1);
		RtlCopyMemory(getData, address, size);
	}
	__except (1)
	{
		bRet = FALSE;
	}
	ObDereferenceObject(process);
	KeUnstackDetachProcess(&stack);
	RtlCopyMemory(res, getData, size);
	ExFreePool(getData);
	return bRet;
}
