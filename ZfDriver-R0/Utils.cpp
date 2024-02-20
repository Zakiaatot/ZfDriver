#include <ntifs.h>
#include <ntimage.h>
#include "Peb.h"
#include "Utils.h"


// Define
#define ToRva(address, offset) address + (INT32)((*(INT32*)(address + offset) + offset) + sizeof(INT32))
#define ToLower(cChar) ((cChar >= 'A' && cChar <= 'Z') ? (cChar + 32) : cChar)

// Export Api
extern "C" {
	NTKERNELAPI PVOID NTAPI PsGetProcessPeb(_In_ PEPROCESS Process);
	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(_In_ PEPROCESS Process);
	NTKERNELAPI PLIST_ENTRY  PsLoadedModuleList;
}

// Type Define
typedef struct _KLDR_DATA_TABLE_ENTRY
{
	/* 0x0000 */ struct _LIST_ENTRY InLoadOrderLinks;
	/* 0x0010 */ void* ExceptionTable;
	/* 0x0018 */ unsigned long ExceptionTableSize;
	/* 0x001c */ long Padding_687;
	/* 0x0020 */ void* GpValue;
	/* 0x0028 */ struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
	/* 0x0030 */ void* DllBase;
	/* 0x0038 */ void* EntryPoint;
	/* 0x0040 */ unsigned long SizeOfImage;
	/* 0x0044 */ long Padding_688;
	/* 0x0048 */ struct _UNICODE_STRING FullDllName;
	/* 0x0058 */ struct _UNICODE_STRING BaseDllName;
	/* 0x0068 */ unsigned long Flags;
	/* 0x006c */ unsigned short LoadCount;
	union
	{
		union
		{
			struct /* bitfield */
			{
				/* 0x006e */ unsigned short SignatureLevel : 4; /* bit position: 0 */
				/* 0x006e */ unsigned short SignatureType : 3; /* bit position: 4 */
				/* 0x006e */ unsigned short Unused : 9; /* bit position: 7 */
			}; /* bitfield */
			/* 0x006e */ unsigned short EntireField;
		}; /* size: 0x0002 */
	} /* size: 0x0002 */ u1;
	/* 0x0070 */ void* SectionPointer;
	/* 0x0078 */ unsigned long CheckSum;
	/* 0x007c */ unsigned long CoverageSectionSize;
	/* 0x0080 */ void* CoverageSection;
	/* 0x0088 */ void* LoadedImports;
	/* 0x0090 */ void* Spare;
	/* 0x0098 */ unsigned long SizeOfImageNotRounded;
	/* 0x009c */ unsigned long TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY; /* size: 0x00a0 */

// GLOBAL
INT64(*gGreProtectSpriteContent)(INT64, UINT64, INT32, CHAR) = NULL;

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
static UNICODE_STRING AnsiToUnicode(PCCHAR str)
{

	UNICODE_STRING unicode;
	ANSI_STRING ansiStr;

	RtlInitAnsiString(&ansiStr, str);
	RtlAnsiStringToUnicodeString(&unicode, &ansiStr, TRUE);

	return unicode;
}
static PKLDR_DATA_TABLE_ENTRY GetLdrDataByName(PCCHAR szmodule)
{
	PKLDR_DATA_TABLE_ENTRY ldrEntry = nullptr;
	UNICODE_STRING mod = AnsiToUnicode(szmodule);

	PLIST_ENTRY psLoadedModuleList = PsLoadedModuleList;
	if (!psLoadedModuleList)
		return ldrEntry;

	auto currentLdrEntry = reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(psLoadedModuleList->Flink);

	while (reinterpret_cast<PLIST_ENTRY>(currentLdrEntry) != psLoadedModuleList)
	{
		if (!RtlCompareUnicodeString(&currentLdrEntry->BaseDllName, &mod, TRUE))
		{
			ldrEntry = currentLdrEntry;
			break;
		}

		currentLdrEntry = reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(currentLdrEntry->InLoadOrderLinks.Flink);
	}

	return ldrEntry;
}
template <typename strType, typename strType2>
__forceinline BOOL crt_strcmp(strType str, strType2 inStr, BOOL two)
{


	if (!str || !inStr)
		return FALSE;

	WCHAR c1, c2;
	do
	{
		c1 = *str++; c2 = *inStr++;
		c1 = ToLower(c1); c2 = ToLower(c2);

		if (!c1 && (two ? !c2 : 1))
			return TRUE;

	} while (c1 == c2);

	return FALSE;
}
static PIMAGE_SECTION_HEADER GetSectionHeader(const PULONGLONG imageBase, PCCHAR sectionName)
{
	if (!imageBase || !sectionName)
		return nullptr;

	const auto pimageDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
	const auto pimageNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS64>(imageBase + pimageDosHeader->e_lfanew);

	auto psection = reinterpret_cast<PIMAGE_SECTION_HEADER>(pimageNtHeader + 1);

	PIMAGE_SECTION_HEADER psectionHdr = nullptr;

	const auto numberOfSections = pimageNtHeader->FileHeader.NumberOfSections;

	for (auto i = 0; i < numberOfSections; ++i)
	{
		if (crt_strcmp(reinterpret_cast<PCCHAR>(psection->Name), sectionName, FALSE))
		{
			psectionHdr = psection;
			break;
		}

		++psection;
	}

	return psectionHdr;
}
static BOOL DataCompare(PCCHAR pdata, PCCHAR bmask, PCCHAR szmask)
{
	for (; *szmask; ++szmask, ++pdata, ++bmask)
	{
		if (*szmask == 'x' && *pdata != *bmask)
			return FALSE;
	}

	return !*szmask;
}
static PULONGLONG FindPattern(const PULONGLONG base, const size_t size, PCCHAR bmask, PCCHAR szmask)
{
	for (size_t i = 0; i < size; ++i)
		if (DataCompare(reinterpret_cast<PCCHAR>(base + i), bmask, szmask))
			return base + i;

	return 0;
}
static PULONGLONG FindPatternPageKm(PCCHAR szmodule, PCCHAR szsection, PCCHAR bmask, PCCHAR szmask)
{
	if (!szmodule || !szsection || !bmask || !szmask)
		return 0;

	const auto* pldrEntry = GetLdrDataByName(szmodule);

	if (!pldrEntry)
		return 0;

	const auto  moduleBase = reinterpret_cast<PULONGLONG>(pldrEntry->DllBase);
	const auto* psection = GetSectionHeader(reinterpret_cast<PULONGLONG>(pldrEntry->DllBase), szsection);

	return psection ? FindPattern(moduleBase + psection->VirtualAddress, psection->Misc.VirtualSize, bmask, szmask) : 0;
}
static BOOL WindowHideInitialize()// 初始化
{
	DbgPrint("[ZfDriver] Window Hide Initializing");
	PVOID greProtectSpriteContentAddress = reinterpret_cast<PVOID>(FindPatternPageKm("win32kfull.sys", ".text",
		"\xE8\x00\x00\x00\x00\x8B\xF8\x85\xC0\x75\x0E", "x????xxxxxx"));

	if (greProtectSpriteContentAddress == 0)
		return FALSE;

	greProtectSpriteContentAddress = reinterpret_cast<PVOID>(ToRva(reinterpret_cast<PULONGLONG>(greProtectSpriteContentAddress), 1));

	*(PVOID*)&gGreProtectSpriteContent = greProtectSpriteContentAddress;

	return TRUE;
}
BOOL Utils::WindowHide(IN HWND hwnd)
{
	// 初始化
	static BOOL init = FALSE;
	if (!init) init = WindowHideInitialize();
	if (init)
	{
		INT attributes = 0x00000011; // WDA_EXCLUDEFROMCAPTURE
		return gGreProtectSpriteContent(0, (INT64)hwnd, 1, attributes);
	}
	else
	{
		DbgPrint("[ZfDriver] Window Hide Initialize Failed");
		return FALSE;
	}
}