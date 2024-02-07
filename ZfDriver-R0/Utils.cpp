#include <ntifs.h>
#include "Utils.h"


BOOL Utils::MDLReadMemory(IN DWORD pid, IN PVOID address, IN DWORD size, OUT BYTE* data)
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
	RtlCopyMemory(data, getData, size);
	ExFreePool(getData);
	return bRet;
}

BOOL Utils::MDLWriteMemory(IN DWORD pid, IN PVOID address, IN DWORD size, IN BYTE* data)
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
	for (int i = 0; i < size; i++)
	{
		getData[i] = data[i];
	}
	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);

	PMDL mdl = IoAllocateMdl(address, size, 0, 0, NULL);
	if (mdl == NULL)
	{
		return FALSE;
	}
	MmBuildMdlForNonPagedPool(mdl);
	BYTE* changeData = NULL;
	__try
	{
		changeData = (BYTE*)MmMapLockedPages(mdl, KernelMode);
		RtlCopyMemory(changeData, getData, size);
	}
	__except (1)
	{
		bRet = FALSE;
		goto END;
	}
END:
	IoFreeMdl(mdl);
	ExFreePool(getData);
	KeUnstackDetachProcess(&stack);
	ObDereferenceObject(process);
	return bRet;
}

// ǿ��ɾ���ļ�
BOOL Utils::ForceDeleteFile(IN UNICODE_STRING pwzFileName)
{
	PEPROCESS pCurEprocess = NULL;
	KAPC_STATE kapc = { 0 };
	OBJECT_ATTRIBUTES fileOb;
	HANDLE hFile = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK iosta;
	PDEVICE_OBJECT DeviceObject = NULL;
	PVOID pHandleFileObject = NULL;

	// �ж��жϵȼ�������0
	if (KeGetCurrentIrql() > PASSIVE_LEVEL)
	{
		return FALSE;
	}
	if (pwzFileName.Buffer == NULL || pwzFileName.Length <= 0)
	{
		return FALSE;
	}

	__try
	{
		// ��ȡ��ǰ���̵�EProcess
		pCurEprocess = IoGetCurrentProcess();

		// ���ӽ���
		KeStackAttachProcess(pCurEprocess, &kapc);

		// ��ʼ���ṹ
		InitializeObjectAttributes(&fileOb, &pwzFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		// �ļ�ϵͳɸѡ���������� ����ָ���豸���������ɸѡ�����ļ�ϵͳ���ʹ�������
		status = IoCreateFileSpecifyDeviceObjectHint(&hFile,
			SYNCHRONIZE | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | FILE_READ_DATA,
			&fileOb,
			&iosta,
			NULL,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			0,
			0,
			CreateFileTypeNone,
			0,
			IO_IGNORE_SHARE_ACCESS_CHECK,
			DeviceObject);
		if (!NT_SUCCESS(status))
		{
			return FALSE;
		}

		// �ڶ��������ṩ������֤����������������Ȩ�ޣ��򷵻�ָ���������ĵ���Ӧָ�롣
		status = ObReferenceObjectByHandle(hFile, 0, 0, 0, &pHandleFileObject, 0);
		if (!NT_SUCCESS(status))
		{
			return FALSE;
		}

		// ����ڶ�������Ϊ0
		((PFILE_OBJECT)(pHandleFileObject))->SectionObjectPointer->ImageSectionObject = 0;

		// ɾ��Ȩ�޴�
		((PFILE_OBJECT)(pHandleFileObject))->DeleteAccess = 1;

		// ����ɾ���ļ�API
		status = ZwDeleteFile(&fileOb);
		if (!NT_SUCCESS(status))
		{
			return FALSE;
		}
	}
	_finally
	{
		if (pHandleFileObject != NULL)
		{
			ObDereferenceObject(pHandleFileObject);
			pHandleFileObject = NULL;
		}
		KeUnstackDetachProcess(&kapc);

		if (hFile != NULL || hFile != (PVOID)-1)
		{
			ZwClose(hFile);
			hFile = (PVOID)-1;
		}
	}
	return TRUE;
}