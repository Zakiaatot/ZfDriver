#include <ntifs.h>
#include <ntimage.h>
#include "Peb.h"
#include "Utils.h"

// Export Api
extern "C" {
	NTKERNELAPI PVOID NTAPI PsGetProcessPeb(_In_ PEPROCESS Process);
	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(_In_ PEPROCESS Process);
	NTSTATUS NTAPI ZwQuerySystemInformation
	(
		DWORD32 systemInformationClass,
		PVOID systemInformation,
		ULONG systemInformationLength,
		PULONG returnLength
	);
	NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(_In_ PVOID ImageBase, _In_ PCCH RoutineName);
}

// Type Define
typedef NTSTATUS(__fastcall* MiProcessLoaderEntry)(PVOID pDriverSection, BOOLEAN bLoad);
typedef INT64(__fastcall* FChangeWindowTreeProtection)(PVOID a1, INT a2);
typedef INT64(__fastcall* FValidateHwnd)(INT64 a1);

typedef struct _SYSTEM_MODULE
{
	ULONG_PTR Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;
typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG_PTR ulModuleCount;
	SYSTEM_MODULE Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

// GLOBAL
FChangeWindowTreeProtection gChangeWindowTreeProtection = 0;
FValidateHwnd gValidateHwnd = 0;

// MDL读
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

// MDL写
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

// 强制删除文件
BOOL Utils::ForceDeleteFile(IN UNICODE_STRING pwzFileName)
{
	PEPROCESS pCurEprocess = NULL;
	KAPC_STATE kapc = { 0 };
	OBJECT_ATTRIBUTES fileOb;
	HANDLE hFile = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK iosta;
	PDEVICE_OBJECT deviceObject = NULL;
	PVOID pHandleFileObject = NULL;

	// 判断中断等级不大于0
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
		// 读取当前进程的EProcess
		pCurEprocess = IoGetCurrentProcess();

		// 附加进程
		KeStackAttachProcess(pCurEprocess, &kapc);

		// 初始化结构
		InitializeObjectAttributes(&fileOb, &pwzFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		// 文件系统筛选器驱动程序 仅向指定设备对象下面的筛选器和文件系统发送创建请求。
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
			deviceObject);
		if (!NT_SUCCESS(status))
		{
			return FALSE;
		}

		// 在对象句柄上提供访问验证，如果可以授予访问权限，则返回指向对象的正文的相应指针。
		status = ObReferenceObjectByHandle(hFile, 0, 0, 0, &pHandleFileObject, 0);
		if (!NT_SUCCESS(status))
		{
			return FALSE;
		}

		// 镜像节对象设置为0
		((PFILE_OBJECT)(pHandleFileObject))->SectionObjectPointer->ImageSectionObject = 0;

		// 删除权限打开
		((PFILE_OBJECT)(pHandleFileObject))->DeleteAccess = 1;

		// 调用删除文件API
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

// WOW64取模块基址
DWORD64 Utils::GetModuleBaseWow64(IN PEPROCESS pEProcess, IN UNICODE_STRING moduleName)
{
	DWORD64 baseAddr = 0;
	KAPC_STATE kapc = { 0 };
	KeStackAttachProcess(pEProcess, &kapc);
	PPEB32 pPeb = (PPEB32)PsGetProcessWow64Process(pEProcess);
	if (pPeb == NULL || pPeb->Ldr == 0)
	{
		KeUnstackDetachProcess(&kapc);
		return 0;
	}

	for (
		PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb->Ldr)->InLoadOrderModuleList.Flink;
		pListEntry != &((PPEB_LDR_DATA32)pPeb->Ldr)->InLoadOrderModuleList;
		pListEntry = (PLIST_ENTRY32)pListEntry->Flink
		)
	{
		PLDR_DATA_TABLE_ENTRY32 ldrEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

		if (ldrEntry->BaseDllName.Buffer == NULL)
		{
			continue;
		}

		// 当前模块名链表
		UNICODE_STRING usCurrentName = { 0 };
		RtlInitUnicodeString(&usCurrentName, (PWCHAR)ldrEntry->BaseDllName.Buffer);

		// 比较模块名是否一致
		if (RtlEqualUnicodeString(&moduleName, &usCurrentName, TRUE))
		{
			baseAddr = (DWORD64)ldrEntry->DllBase;
			KeUnstackDetachProcess(&kapc);
			return baseAddr;
		}
	}
	KeUnstackDetachProcess(&kapc);
	return 0;
}

// X64取模块基址
DWORD64 Utils::GetModuleBase64(IN PEPROCESS pEProcess, IN UNICODE_STRING moduleName)
{
	DWORDLONG baseAddr = 0;
	KAPC_STATE kapc = { 0 };
	KeStackAttachProcess(pEProcess, &kapc);
	PPEB64 pPeb = (PPEB64)PsGetProcessPeb(pEProcess);
	if (pPeb == NULL || pPeb->Ldr == NULL)
	{
		KeUnstackDetachProcess(&kapc);
		return 0;
	}

	for (
		PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink;
		pListEntry != &(pPeb->Ldr->InLoadOrderModuleList);
		pListEntry = pListEntry->Flink
		)
	{
		PLDR_DATA_TABLE_ENTRY ldrEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (ldrEntry->BaseDllName.Buffer == NULL)
		{
			continue;
		}

		// 当前模块名链表
		UNICODE_STRING usCurrentName = { 0 };
		RtlInitUnicodeString(&usCurrentName, (PWCHAR)ldrEntry->BaseDllName.Buffer);

		if (RtlEqualUnicodeString(&moduleName, &usCurrentName, TRUE))
		{
			baseAddr = (DWORD64)ldrEntry->DllBase;
			KeUnstackDetachProcess(&kapc);
			return baseAddr;
		}
	}
	KeUnstackDetachProcess(&kapc);
	return 0;
}

// 进程隐藏
static DWORD FindEprocessActiveProcessLinksOffset() // 获取ActiveProcessLinks偏移
{
	DWORD ofs = 0; // The offset we're looking for
	int idx = 0;                // Index 
	DWORD pids[3];				// List of PIDs for our 3 processes
	PEPROCESS eprocs[3];		// Process list, will contain 3 processes

	//Select 3 process PIDs and get their EPROCESS Pointer
	for (int i = 16; idx < 3; i += 4)
	{
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)i, &eprocs[idx])))
		{
			pids[idx] = i;
			idx++;
		}
	}

	/*
	Go through the EPROCESS structure and look for the PID
	we can start at 0x20 because UniqueProcessId should
	not be in the first 0x20 bytes,
	also we should stop after 0x600 bytes with no success
	*/

	for (int i = 0x20; i < 0x600; i += 4)
	{
		if ((*(DWORD*)((UCHAR*)eprocs[0] + i) == pids[0])
			&& (*(DWORD*)((UCHAR*)eprocs[1] + i) == pids[1])
			&& (*(DWORD*)((UCHAR*)eprocs[2] + i) == pids[2]))
		{
			//+ 0x440 UniqueProcessId  : Ptr64 Void
			//+ 0x448 ActiveProcessLinks : _LIST_ENTRY
			// UniqueProcessId next is ActiveProcessLinks 
			ofs = i + sizeof(PVOID);
			break;
		}
	}

	ObDereferenceObject(eprocs[0]);
	ObDereferenceObject(eprocs[1]);
	ObDereferenceObject(eprocs[2]);
	DbgPrint("[ZfDriver] Activeprocesslinks Offset: 0x%x", ofs);
	return ofs;
}
BOOL Utils::ProcessHide(IN DWORD pid)
{
	PEPROCESS pEProcess = NULL;

	PsLookupProcessByProcessId((HANDLE)pid, &pEProcess);
	if (pEProcess == NULL)
	{
		return FALSE;
	}
	static DWORD offset = FindEprocessActiveProcessLinksOffset();
	PLIST_ENTRY listEntry = (PLIST_ENTRY)((DWORD64)pEProcess + offset);

	listEntry->Flink->Blink = listEntry->Blink;
	listEntry->Blink->Flink = listEntry->Flink;
	listEntry->Flink = listEntry;
	listEntry->Blink = listEntry;

	return TRUE;
}

// 窗口隐藏
static BOOL GetModuleBaseAddress(PCCHAR name, ULONGLONG& addr, ULONG& size)// 获取模块基址
{
	ULONG needSize = 0;
	ZwQuerySystemInformation(11, &needSize, 0, &needSize);
	if (needSize == 0) return FALSE;

	ULONG tag = 'VMON';
	PSYSTEM_MODULE_INFORMATION sysMods = (PSYSTEM_MODULE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, needSize, tag);
	if (sysMods == 0) return FALSE;

	NTSTATUS status = ZwQuerySystemInformation(11, sysMods, needSize, 0);
	if (!NT_SUCCESS(status))
	{
		ExFreePoolWithTag(sysMods, tag);
		return FALSE;
	}

	for (ULONGLONG i = 0; i < sysMods->ulModuleCount; i++)
	{
		PSYSTEM_MODULE mod = &sysMods->Modules[i];
		if (strstr(mod->ImageName, name))
		{
			addr = (ULONGLONG)mod->Base;
			size = (ULONG)mod->Size;
			break;
		}
	}

	ExFreePoolWithTag(sysMods, tag);
	return TRUE;
}
static BOOL PatternCheck(PCCHAR data, PCCHAR pattern, PCCHAR mask)// 模式匹配
{
	size_t len = strlen(mask);

	for (size_t i = 0; i < len; i++)
	{
		if (data[i] == pattern[i] || mask[i] == '?')
			continue;
		else
			return FALSE;
	}

	return TRUE;
}
static ULONGLONG FindPattern(ULONGLONG addr, ULONG size, PCCHAR pattern, PCCHAR mask)
{
	size -= (ULONG)strlen(mask);

	for (ULONG i = 0; i < size; i++)
	{
		if (PatternCheck((PCCHAR)addr + i, pattern, mask))
			return addr + i;
	}

	return 0;
}
static ULONGLONG FindPatternImage(ULONGLONG addr, PCCHAR pattern, PCCHAR mask)
{
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)addr;
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	PIMAGE_NT_HEADERS64 nt = (PIMAGE_NT_HEADERS64)(addr + dos->e_lfanew);
	if (nt->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);

	for (USHORT i = 0; i < nt->FileHeader.NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER p = &section[i];

		if (strstr((PCCHAR)p->Name, ".text") || 'EGAP' == *reinterpret_cast<int*>(p->Name))
		{
			ULONGLONG res = FindPattern(addr + p->VirtualAddress, p->Misc.VirtualSize, pattern, mask);
			if (res) return res;
		}
	}

	return 0;
}
static PVOID GetSystemBaseExport(PCCHAR moduleName, PCCHAR routineName)// 获取导出函数
{
	ULONGLONG win32kbaseAddress = 0;
	ULONG win32kbaseLength = 0;
	GetModuleBaseAddress(moduleName, win32kbaseAddress, win32kbaseLength);
	DbgPrintEx(0, 0, "[ZfDriver] %s base address is 0x%llX \n", moduleName, win32kbaseAddress);
	if (MmIsAddressValid((PVOID)win32kbaseAddress) == FALSE) return 0;

	return RtlFindExportedRoutineByName((PVOID)win32kbaseAddress, routineName);
}
static BOOL WindowHideInitialize()// 初始化
{
	DbgPrint("[ZfDriver] Window Hide Initializing");
	ULONGLONG win32kfullAddress = 0;
	ULONG win32kfullLength = 0;
	GetModuleBaseAddress("win32kfull.sys", win32kfullAddress, win32kfullLength);
	DbgPrintEx(0, 0, "[ZfDriver] win32kfull base address is 0x%llX \n", win32kfullAddress);
	if (MmIsAddressValid((PVOID)win32kfullAddress) == FALSE) return FALSE;

	/*
	call    ?ChangeWindowTreeProtection@@YAHPEAUtagWND@@H@Z ; ChangeWindowTreeProtection(tagWND *,int)
	mov     esi, eax
	test    eax, eax
	jnz     short loc_1C0245002
	*/
	ULONGLONG address = FindPatternImage(win32kfullAddress,
		"\xE8\x00\x00\x00\x00\x8B\xF0\x85\xC0\x75\x00\x44\x8B\x44",
		"x????x?xxx?xxx");
	DbgPrintEx(0, 0, "[ZfDriver] Pattern address is 0x%llX \n", address);
	if (address == 0) return FALSE;

	// 5=汇编指令长度
	// 1=偏移
	gChangeWindowTreeProtection = reinterpret_cast<FChangeWindowTreeProtection>(reinterpret_cast<PCHAR>(address) + 5 + *reinterpret_cast<PINT>(reinterpret_cast<char*>(address) + 1));
	DbgPrintEx(0, 0, "[ZfDriver] ChangeWindowTreeProtection address is 0x%p \n", gChangeWindowTreeProtection);
	if (MmIsAddressValid(gChangeWindowTreeProtection) == FALSE) return FALSE;

	gValidateHwnd = (FValidateHwnd)GetSystemBaseExport("win32kbase.sys", "ValidateHwnd");
	DbgPrintEx(0, 0, "[ZfDriver] ValidateHwnd address is 0x%p \n", gValidateHwnd);
	if (MmIsAddressValid(gValidateHwnd) == FALSE) return FALSE;

	return TRUE;
}
static INT64 ChangeWindowAttributes(INT64 handler, INT attributes)// 修改窗口状态
{
	if (MmIsAddressValid(gChangeWindowTreeProtection) == FALSE) return 0;
	if (MmIsAddressValid(gValidateHwnd) == FALSE) return 0;

	PVOID wndPtr = (PVOID)gValidateHwnd(handler);
	if (MmIsAddressValid(wndPtr) == FALSE) return 0;

	return gChangeWindowTreeProtection(wndPtr, attributes);
}
BOOL Utils::WindowHide(IN HWND hwnd)
{
	// 初始化
	static BOOL init = FALSE;
	if (!init) init = WindowHideInitialize();
	if (init)
	{
		INT attributes = 0x00000011; // WDA_EXCLUDEFROMCAPTURE
		return ChangeWindowAttributes((INT64)hwnd, attributes);
	}
	else
	{
		DbgPrint("[ZfDriver] Window Hide Initialize Failed");
		return FALSE;
	}
}