#include <ntifs.h>
#include <ntimage.h>
#include "peb.h"
#include "Utils.h"


// Define
#define ToRva(address, offset) address + (INT32)((*(INT32*)(address + offset) + offset) + sizeof(INT32))
#define ToLower(cChar) ((cChar >= 'A' && cChar <= 'Z') ? (cChar + 32) : cChar)

// Export Api
extern "C" {
	NTKERNELAPI PVOID NTAPI PsGetProcessPeb(_In_ PEPROCESS Process);
	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(_In_ PEPROCESS Process);
	NTKERNELAPI PLIST_ENTRY  PsLoadedModuleList;
	NTKERNELAPI NTSTATUS ZwQuerySystemInformation
	(
		ULONG SystemInformationClass,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength
	);
	NTKERNELAPI NTSTATUS RtlCreateUserThread
	(
		HANDLE ProcessHandle,
		PSECURITY_DESCRIPTOR SecurityDescriptor,
		BOOLEAN CreateSuspended,
		ULONG StackZeroBits,
		SIZE_T StackReserve,
		SIZE_T StackCommit,
		PVOID startAddress,
		PVOID StartParameter,
		PHANDLE ThreadHandle,
		PVOID ClientID
	);
}

// type Define
typedef struct _KLDR_DATA_TABLE_ENTRY
{
	/* 0x0000 */ struct _LIST_ENTRY InLoadOrderLinks;
	/* 0x0010 */ void* ExceptionTable;
	/* 0x0018 */ unsigned long ExceptionTableSize;
	/* 0x001c */ long Padding_687;
	/* 0x0020 */ void* GpValue;
	/* 0x0028 */ struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
	/* 0x0030 */ void* dllBase;
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
typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation = 0x0,
	SystemProcessorInformation = 0x1,
	SystemPerformanceInformation = 0x2,
	SystemTimeOfDayInformation = 0x3,
	SystemPathInformation = 0x4,
	SystemProcessInformation = 0x5,
	SystemCallCountInformation = 0x6,
	SystemDeviceInformation = 0x7,
	SystemProcessorPerformanceInformation = 0x8,
	SystemFlagsInformation = 0x9,
	SystemCallTimeInformation = 0xa,
	SystemModuleInformation = 0xb,
	SystemLocksInformation = 0xc,
	SystemStackTraceInformation = 0xd,
	SystemPagedPoolInformation = 0xe,
	SystemNonPagedPoolInformation = 0xf,
	SystemHandleInformation = 0x10,
	SystemObjectInformation = 0x11,
	SystemPageFileInformation = 0x12,
	SystemVdmInstemulInformation = 0x13,
	SystemVdmBopInformation = 0x14,
	SystemFileCacheInformation = 0x15,
	SystemPoolTagInformation = 0x16,
	SystemInterruptInformation = 0x17,
	SystemDpcBehaviorInformation = 0x18,
	SystemFullMemoryInformation = 0x19,
	SystemLoadGdiDriverInformation = 0x1a,
	SystemUnloadGdiDriverInformation = 0x1b,
	SystemTimeAdjustmentInformation = 0x1c,
	SystemSummaryMemoryInformation = 0x1d,
	SystemMirrorMemoryInformation = 0x1e,
	SystemPerformanceTraceInformation = 0x1f,
	SystemObsolete0 = 0x20,
	SystemExceptionInformation = 0x21,
	SystemCrashDumpStateInformation = 0x22,
	SystemKernelDebuggerInformation = 0x23,
	SystemContextSwitchInformation = 0x24,
	SystemRegistryQuotaInformation = 0x25,
	SystemExtendServiceTableInformation = 0x26,
	SystemPrioritySeperation = 0x27,
	SystemVerifierAddDriverInformation = 0x28,
	SystemVerifierRemoveDriverInformation = 0x29,
	SystemProcessorIdleInformation = 0x2a,
	SystemLegacyDriverInformation = 0x2b,
	SystemCurrentTimeZoneInformation = 0x2c,
	SystemLookasideInformation = 0x2d,
	SystemTimeSlipNotification = 0x2e,
	SystemSessionCreate = 0x2f,
	SystemSessionDetach = 0x30,
	SystemSessionInformation = 0x31,
	SystemRangeStartInformation = 0x32,
	SystemVerifierInformation = 0x33,
	SystemVerifierThunkExtend = 0x34,
	SystemSessionProcessInformation = 0x35,
	SystemLoadGdiDriverInSystemSpace = 0x36,
	SystemNumaProcessorMap = 0x37,
	SystemPrefetcherInformation = 0x38,
	SystemExtendedProcessInformation = 0x39,
	SystemRecommendedSharedDataAlignment = 0x3a,
	SystemComPlusPackage = 0x3b,
	SystemNumaAvailableMemory = 0x3c,
	SystemProcessorPowerInformation = 0x3d,
	SystemEmulationBasicInformation = 0x3e,
	SystemEmulationProcessorInformation = 0x3f,
	SystemExtendedHandleInformation = 0x40,
	SystemLostDelayedWriteInformation = 0x41,
	SystemBigPoolInformation = 0x42,
	SystemSessionPoolTagInformation = 0x43,
	SystemSessionMappedViewInformation = 0x44,
	SystemHotpatchInformation = 0x45,
	SystemObjectSecurityMode = 0x46,
	SystemWatchdogTimerHandler = 0x47,
	SystemWatchdogTimerInformation = 0x48,
	SystemLogicalProcessorInformation = 0x49,
	SystemWow64SharedInformationObsolete = 0x4a,
	SystemRegisterFirmwareTableInformationHandler = 0x4b,
	SystemFirmwareTableInformation = 0x4c,
	SystemModuleInformationEx = 0x4d,
	SystemVerifierTriageInformation = 0x4e,
	SystemSuperfetchInformation = 0x4f,
	SystemMemoryListInformation = 0x50,
	SystemFileCacheInformationEx = 0x51,
	SystemThreadPriorityClientIdInformation = 0x52,
	SystemProcessorIdleCycleTimeInformation = 0x53,
	SystemVerifierCancellationInformation = 0x54,
	SystemProcessorPowerInformationEx = 0x55,
	SystemRefTraceInformation = 0x56,
	SystemSpecialPoolInformation = 0x57,
	SystemProcessIdInformation = 0x58,
	SystemErrorPortInformation = 0x59,
	SystemBootEnvironmentInformation = 0x5a,
	SystemHypervisorInformation = 0x5b,
	SystemVerifierInformationEx = 0x5c,
	SystemTimeZoneInformation = 0x5d,
	SystemImageFileExecutionOptionsInformation = 0x5e,
	SystemCoverageInformation = 0x5f,
	SystemPrefetchPatchInformation = 0x60,
	SystemVerifierFaultsInformation = 0x61,
	SystemSystemPartitionInformation = 0x62,
	SystemSystemDiskInformation = 0x63,
	SystemProcessorPerformanceDistribution = 0x64,
	SystemNumaProximityNodeInformation = 0x65,
	SystemDynamicTimeZoneInformation = 0x66,
	SystemCodeIntegrityInformation = 0x67,
	SystemProcessorMicrocodeUpdateInformation = 0x68,
	SystemProcessorBrandString = 0x69,
	SystemVirtualAddressInformation = 0x6a,
	SystemLogicalProcessorAndGroupInformation = 0x6b,
	SystemProcessorCycleTimeInformation = 0x6c,
	SystemStoreInformation = 0x6d,
	SystemRegistryAppendString = 0x6e,
	SystemAitSamplingValue = 0x6f,
	SystemVhdBootInformation = 0x70,
	SystemCpuQuotaInformation = 0x71,
	SystemNativeBasicInformation = 0x72,
	SystemErrorPortTimeouts = 0x73,
	SystemLowPriorityIoInformation = 0x74,
	SystemBootEntropyInformation = 0x75,
	SystemVerifierCountersInformation = 0x76,
	SystemPagedPoolInformationEx = 0x77,
	SystemSystemPtesInformationEx = 0x78,
	SystemNodeDistanceInformation = 0x79,
	SystemAcpiAuditInformation = 0x7a,
	SystemBasicPerformanceInformation = 0x7b,
	SystemQueryPerformanceCounterInformation = 0x7c,
	SystemSessionBigPoolInformation = 0x7d,
	SystemBootGraphicsInformation = 0x7e,
	SystemScrubPhysicalMemoryInformation = 0x7f,
	SystemBadPageInformation = 0x80,
	SystemProcessorProfileControlArea = 0x81,
	SystemCombinePhysicalMemoryInformation = 0x82,
	SystemEntropyInterruptTimingInformation = 0x83,
	SystemConsoleInformation = 0x84,
	SystemPlatformBinaryInformation = 0x85,
	SystemThrottleNotificationInformation = 0x86,
	SystemHypervisorProcessorCountInformation = 0x87,
	SystemDeviceDataInformation = 0x88,
	SystemDeviceDataEnumerationInformation = 0x89,
	SystemMemoryTopologyInformation = 0x8a,
	SystemMemoryChannelInformation = 0x8b,
	SystemBootLogoInformation = 0x8c,
	SystemProcessorPerformanceInformationEx = 0x8d,
	SystemSpare0 = 0x8e,
	SystemSecureBootPolicyInformation = 0x8f,
	SystemPageFileInformationEx = 0x90,
	SystemSecureBootInformation = 0x91,
	SystemEntropyInterruptTimingRawInformation = 0x92,
	SystemPortableWorkspaceEfiLauncherInformation = 0x93,
	SystemFullProcessInformation = 0x94,
	SystemKernelDebuggerInformationEx = 0x95,
	SystemBootMetadataInformation = 0x96,
	SystemSoftRebootInformation = 0x97,
	SystemElamCertificateInformation = 0x98,
	SystemOfflineDumpConfigInformation = 0x99,
	SystemProcessorFeaturesInformation = 0x9a,
	SystemRegistryReconciliationInformation = 0x9b,
	MaxSystemInfoClass = 0x9c,
} SYSTEM_INFORMATION_CLASS;
typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset; //因次结构是链表结构,所以此成员记录了下一此结构的偏移
	ULONG NumberOfThreads;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER SpareLi2;
	LARGE_INTEGER SpareLi3;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName; //记录的进程名
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;              //记录的进程ID
	HANDLE InheritedFromUniqueProcessId; //父进程ID
	ULONG HandleCount;
	ULONG SessionId; //会话ID
	ULONG_PTR PageDirectoryBase;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;   //记录了虚拟大小
	ULONG PageFaultCount; //记录了错误页的个数
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

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
		// 读取当前进程的eProcess
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
	int idx = 0;                // index 
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
	DbgPrint("[ZfDriver] Window Hide: GetLdrDataByName");
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
__forceinline BOOL CrtStrcmp(strType str, strType2 inStr, BOOL two)
{
	DbgPrint("[ZfDriver] Window Hide: CrtStrcmp");
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
static PIMAGE_SECTION_HEADER GetSectionHeader(const ULONGLONG imageBase, PCCHAR sectionName)
{
	DbgPrint("[ZfDriver] Window Hide: GetSectionHeader");
	if (!imageBase || !sectionName)
		return nullptr;

	const auto pimageDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
	const auto pimageNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS64>(imageBase + pimageDosHeader->e_lfanew);

	auto psection = reinterpret_cast<PIMAGE_SECTION_HEADER>(pimageNtHeader + 1);

	PIMAGE_SECTION_HEADER psectionHdr = nullptr;

	const auto numberOfSections = pimageNtHeader->FileHeader.NumberOfSections;

	for (auto i = 0; i < numberOfSections; ++i)
	{
		if (CrtStrcmp(reinterpret_cast<PCCHAR>(psection->Name), sectionName, FALSE))
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
static ULONGLONG FindPattern(const ULONGLONG base, const size_t size, PCCHAR bmask, PCCHAR szmask)
{
	DbgPrint("[ZfDriver] Window Hide: FindPatternPageKm");
	for (size_t i = 0; i < size; ++i)
		if (DataCompare(reinterpret_cast<PCCHAR>(base + i), bmask, szmask))
			return base + i;

	return 0;
}
static ULONGLONG FindPatternPageKm(PCCHAR szmodule, PCCHAR szsection, PCCHAR bmask, PCCHAR szmask)
{
	DbgPrint("[ZfDriver] Window Hide: FindPatternPageKm");
	if (!szmodule || !szsection || !bmask || !szmask)
		return 0;

	const auto* pldrEntry = GetLdrDataByName(szmodule);

	if (!pldrEntry)
		return 0;

	const auto  moduleBase = reinterpret_cast<ULONGLONG>(pldrEntry->dllBase);
	const auto* psection = GetSectionHeader(reinterpret_cast<ULONGLONG>(pldrEntry->dllBase), szsection);

	return psection ? FindPattern(moduleBase + psection->VirtualAddress, psection->Misc.VirtualSize, bmask, szmask) : 0;
}
static BOOL WindowHideInitialize()// 初始化
{
	DbgPrint("[ZfDriver] Window Hide Initializing");
	PVOID greProtectSpriteContentAddress = reinterpret_cast<PVOID>(FindPatternPageKm("win32kfull.sys", ".text",
		"\xE8\x00\x00\x00\x00\x8B\xF8\x85\xC0\x75\x0E", "x????xxxxxx"));

	if (greProtectSpriteContentAddress == 0)
		return FALSE;

	greProtectSpriteContentAddress = reinterpret_cast<PVOID>(ToRva(reinterpret_cast<ULONGLONG>(greProtectSpriteContentAddress), 1));

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

// 根据进程名获取进程ID
DWORD Utils::GetProcessId(UNICODE_STRING processName)
{

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSYSTEM_PROCESS_INFORMATION info = NULL;
	ULONG writeBytes = 0;
	PVOID freeAddress = NULL;
	ULONG64 processId = 0;
	BOOLEAN unFree = FALSE;
	__try
	{
		status = ZwQuerySystemInformation(SystemProcessInformation, 0, 0, &writeBytes);
		while (status == STATUS_INFO_LENGTH_MISMATCH)
		{
			if (info != NULL)
			{
				ExFreePoolWithTag(info, 'QsIn');
				unFree = FALSE;
				info = NULL;
			}
			info = (PSYSTEM_PROCESS_INFORMATION)ExAllocatePoolPriorityUninitialized(PagedPool, writeBytes, 'QsIn', HighPoolPriority);
			if (info == NULL)
				return 0;
			unFree = TRUE;
			RtlZeroMemory(info, writeBytes);
			status = ZwQuerySystemInformation(SystemProcessInformation, info, writeBytes, &writeBytes);
		}
		freeAddress = info;
		unFree = FALSE;
		while (info->NextEntryOffset != 0)
		{
			if (info != NULL)
			{
				if (RtlEqualUnicodeString(&info->ImageName, &processName, TRUE))
				{
					RtlCopyMemory(&processId, &info->UniqueProcessId, sizeof(ULONG64));
					break;
				}
			}
			info = (PSYSTEM_PROCESS_INFORMATION)((ULONG64)info + info->NextEntryOffset);
		}
		if (freeAddress != NULL)
			ExFreePoolWithTag(freeAddress, 'QsIn');
		return (DWORD)processId;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (freeAddress != NULL && unFree == FALSE)
			ExFreePoolWithTag(freeAddress, 'QsIn');
		else if (info != NULL && unFree == TRUE)
			ExFreePoolWithTag(info, 'QsIn');
		return 0;
	}
}

// 申请内存
PVOID Utils::NewMem(HANDLE processId, ULONG size)
{
	// 检查进程ID是否为系统进程
	if (processId == 0 || processId == (HANDLE)4)
		return nullptr;
	PEPROCESS tProcess = NULL;
	BOOLEAN isDereferenceObject = FALSE;
	BOOLEAN isAttachProcess = FALSE;
	KAPC_STATE apcState = { 0 };
	__try
	{
		// 获取进程信息
		if (!NT_SUCCESS(PsLookupProcessByProcessId(processId, &tProcess)))
			return nullptr;
		isDereferenceObject = TRUE;
		ObDereferenceObject(tProcess);
		isDereferenceObject = FALSE;
		KeStackAttachProcess(tProcess, &apcState);
		isAttachProcess = TRUE;
		PVOID address = 0;
		SIZE_T allocSize = size;
		ZwAllocateVirtualMemory(NtCurrentProcess(), &address, 0, &allocSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		KeUnstackDetachProcess(&apcState);
		isAttachProcess = FALSE;
		if (address == 0)
			return nullptr;
		else
			return address;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (isAttachProcess)
			KeUnstackDetachProcess(&apcState);
		if (isDereferenceObject)
			ObDereferenceObject(tProcess);
		return nullptr;
	}
}

// 释放内存
BOOL Utils::DelMem(HANDLE processId, PVOID address)
{
	// 检查进程ID是否为系统进程
	if (processId == 0 || processId == (HANDLE)4)
		return FALSE;
	// 检查地址
	if ((ULONG64)address < 0x10000 || (ULONG64)address > 0x7FFFFFFFFFFF)
		return FALSE;
	PEPROCESS tProcess = NULL;
	BOOLEAN isDereferenceObject = FALSE;
	BOOLEAN isAttachProcess = FALSE;
	KAPC_STATE apcState = { 0 };
	__try
	{
		// 获取进程信息
		if (!NT_SUCCESS(PsLookupProcessByProcessId(processId, &tProcess)))
			return FALSE;
		isDereferenceObject = TRUE;
		ObDereferenceObject(tProcess);
		isDereferenceObject = FALSE;
		KeStackAttachProcess(tProcess, &apcState);
		isAttachProcess = TRUE;
		SIZE_T freeSize = 0;
		NTSTATUS status = ZwFreeVirtualMemory(NtCurrentProcess(), &address, &freeSize, MEM_RELEASE);
		KeUnstackDetachProcess(&apcState);
		isAttachProcess = FALSE;
		if (!NT_SUCCESS(status))
			return FALSE;
		else
			return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (isAttachProcess)
			KeUnstackDetachProcess(&apcState);
		if (isDereferenceObject)
			ObDereferenceObject(tProcess);
		return FALSE;
	}
}

// 创建远程线程
BOOL Utils::CreateThread(HANDLE processId, PVOID startAddress, PVOID parmaAddress)
{
	// 检查进程ID是否为系统进程
	if (processId == 0 || processId == (HANDLE)4)
		return FALSE;
	// 检查地址
	if ((ULONG64)startAddress < 0x10000 || (ULONG64)startAddress > 0x7FFFFFFFFFFF)
		return FALSE;
	if ((ULONG64)parmaAddress > 0x7FFFFFFFFFFF)   // 参数地址可以为NULL
		return FALSE;
	// 获取进程信息
	PEPROCESS tProcess = NULL;
	KAPC_STATE apcState = { 0 };
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HANDLE threadHandle = NULL;
	BOOLEAN isDereferenceObject = FALSE;
	BOOLEAN isAttachProcess = FALSE;
	BOOLEAN isCloseHandle = FALSE;
	__try
	{
		if (!NT_SUCCESS(PsLookupProcessByProcessId(processId, &tProcess)))
			return FALSE;
		ObDereferenceObject(tProcess);
		isDereferenceObject = TRUE;
		KeStackAttachProcess(tProcess, &apcState);
		isAttachProcess = TRUE;
		status = RtlCreateUserThread(NtCurrentProcess(), NULL, FALSE, 0, 0, 0, startAddress, parmaAddress, &threadHandle, NULL);
		if (NT_SUCCESS(status))
		{
			LARGE_INTEGER time = { 0 };
			time.QuadPart = -(60ll * 10 * 1000 * 100);
			ZwWaitForSingleObject(threadHandle, TRUE, &time);
			// 关闭线程句柄
			isCloseHandle = TRUE;
			ZwClose(threadHandle);
			isCloseHandle = FALSE;
		}
		KeUnstackDetachProcess(&apcState);
		isAttachProcess = FALSE;
		ObDereferenceObject(tProcess);
		isDereferenceObject = FALSE;
		if (!NT_SUCCESS(status))
			return FALSE;
		else
			return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (isCloseHandle)
			ZwClose(threadHandle);
		if (isAttachProcess)
			KeUnstackDetachProcess(&apcState);
		if (isDereferenceObject)
			ObDereferenceObject(tProcess);
		return FALSE;
	}
}

// DLL远程线程注入
static PVOID GetModuleExportAddress(IN PVOID moduleBase, IN PCCHAR functionName, IN PEPROCESS eProcess)
{
	PIMAGE_DOS_HEADER imageDosHeader = (PIMAGE_DOS_HEADER)moduleBase;
	PIMAGE_NT_HEADERS32 imageNtHeaders32 = NULL;
	PIMAGE_NT_HEADERS64 imageNtHeaders64 = NULL;
	PIMAGE_EXPORT_DIRECTORY imageExportDirectory = NULL;
	ULONG exportDirectorySize = 0;
	ULONG_PTR functionAddress = 0;

	if (moduleBase == NULL)
	{
		return NULL;
	}

	if (imageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}

	imageNtHeaders32 = (PIMAGE_NT_HEADERS32)((PUCHAR)moduleBase + imageDosHeader->e_lfanew);
	imageNtHeaders64 = (PIMAGE_NT_HEADERS64)((PUCHAR)moduleBase + imageDosHeader->e_lfanew);

	// 判断PE结构位数
	if (imageNtHeaders64->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(imageNtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)moduleBase);
		exportDirectorySize = imageNtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	}
	else
	{
		imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(imageNtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)moduleBase);
		exportDirectorySize = imageNtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	}

	// 解析内存导出表
	PUSHORT pAddressOfOrds = (PUSHORT)(imageExportDirectory->AddressOfNameOrdinals + (ULONG_PTR)moduleBase);
	PULONG  pAddressOfNames = (PULONG)(imageExportDirectory->AddressOfNames + (ULONG_PTR)moduleBase);
	PULONG  pAddressOfFuncs = (PULONG)(imageExportDirectory->AddressOfFunctions + (ULONG_PTR)moduleBase);

	for (ULONG i = 0; i < imageExportDirectory->NumberOfFunctions; ++i)
	{
		USHORT ordIndex = 0xFFFF;
		PCHAR  pName = NULL;

		// 如果函数名小于等于0xFFFF 则说明是序号导出
		if ((ULONG_PTR)functionName <= 0xFFFF)
		{
			ordIndex = (USHORT)i;
		}

		// 否则则说明是名字导出
		else if ((ULONG_PTR)functionName > 0xFFFF && i < imageExportDirectory->NumberOfNames)
		{
			pName = (PCHAR)(pAddressOfNames[i] + (ULONG_PTR)moduleBase);
			ordIndex = pAddressOfOrds[i];
		}

		// 未知导出函数
		else
		{
			return NULL;
		}

		// 对比模块名是否是我们所需要的
		if (((ULONG_PTR)functionName <= 0xFFFF && (USHORT)((ULONG_PTR)functionName) == ordIndex + imageExportDirectory->Base) || ((ULONG_PTR)functionName > 0xFFFF && strcmp(pName, functionName) == 0))
		{
			// 是则保存下来
			functionAddress = pAddressOfFuncs[ordIndex] + (ULONG_PTR)moduleBase;
			break;
		}
	}
	return (PVOID)functionAddress;
}
static PVOID GetUserModuleAddress(IN PEPROCESS eProcess, IN PUNICODE_STRING moduleName, IN BOOLEAN isWow64)
{
	if (eProcess == NULL)
	{
		return NULL;
	}

	__try
	{
		// 定时250ms毫秒
		LARGE_INTEGER time = { 0 };
		time.QuadPart = -250ll * 10 * 1000;

		// 32位执行
		if (isWow64)
		{
			// 得到进程PEB进程环境块
			PPEB32 peb32 = (PPEB32)PsGetProcessWow64Process(eProcess);
			if (peb32 == NULL)
			{
				return NULL;
			}

			// 等待 250ms * 10
			for (INT i = 0; !peb32->Ldr && i < 10; i++)
			{
				// 等待一会在执行
				KeDelayExecutionThread(KernelMode, TRUE, &time);
			}

			// 没有找到返回空
			if (!peb32->Ldr)
			{
				return NULL;
			}

			// 搜索 InLoadOrderModuleList
			for (PLIST_ENTRY32 listEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)peb32->Ldr)->InLoadOrderModuleList.Flink; listEntry != &((PPEB_LDR_DATA32)peb32->Ldr)->InLoadOrderModuleList; listEntry = (PLIST_ENTRY32)listEntry->Flink)
			{
				UNICODE_STRING unicodeString;
				PLDR_DATA_TABLE_ENTRY32 ldrDataTableEntry32 = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);
				RtlInitUnicodeString(&unicodeString, (PWCH)ldrDataTableEntry32->BaseDllName.Buffer);

				// 判断模块名是否符合要求
				if (RtlCompareUnicodeString(&unicodeString, moduleName, TRUE) == 0)
				{
					// 符合则返回模块基址
					return (PVOID)ldrDataTableEntry32->DllBase;
				}
			}
		}

		// 64位执行
		else
		{
			// 得到进程PEB进程环境块
			PPEB64 peb = (PPEB64)PsGetProcessPeb(eProcess);
			if (!peb)
			{
				return NULL;
			}

			// 等待
			for (INT i = 0; !peb->Ldr && i < 10; i++)
			{
				// 将当前线程置于指定间隔的可警报或不可操作的等待状态
				KeDelayExecutionThread(KernelMode, TRUE, &time);
			}
			if (!peb->Ldr)
			{
				return NULL;
			}

			// 遍历链表
			for (PLIST_ENTRY listEntry = peb->Ldr->InLoadOrderModuleList.Flink; listEntry != &peb->Ldr->InLoadOrderModuleList; listEntry = listEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY ldrDataTableEntry = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

				// 判断模块名是否符合要求
				if (RtlCompareUnicodeString(&ldrDataTableEntry->BaseDllName, moduleName, TRUE) == 0)
				{
					// 返回模块基址
					return ldrDataTableEntry->DllBase;
				}
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}

	return NULL;
}
static PVOID GetProcessAddress(HANDLE processId, PWCHAR dllName, PCCHAR functionName)
{
	PEPROCESS eProcess = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	KAPC_STATE apcState;
	PVOID refAddress = 0;

	// 根据PID得到进程eProcess结构
	status = PsLookupProcessByProcessId(processId, &eProcess);
	if (status != STATUS_SUCCESS)
	{
		return 0;
	}

	// 判断目标进程是32位还是64位
	BOOLEAN isWow64 = (PsGetProcessWow64Process(eProcess) != NULL) ? TRUE : FALSE;

	// 验证地址是否可读
	if (!MmIsAddressValid(eProcess))
	{
		return NULL;
	}

	// 将当前线程连接到目标进程的地址空间(附加进程)
	KeStackAttachProcess((PRKPROCESS)eProcess, &apcState);

	__try
	{
		UNICODE_STRING dllUnicodeString = { 0 };
		PVOID baseAddress = NULL;

		// 得到进程内模块基地址
		RtlInitUnicodeString(&dllUnicodeString, dllName);

		baseAddress = GetUserModuleAddress(eProcess, &dllUnicodeString, isWow64);

		if (!baseAddress)
		{
			return NULL;
		}

		// 得到该函数地址
		refAddress = GetModuleExportAddress(baseAddress, functionName, eProcess);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}

	// 取消附加
	KeUnstackDetachProcess(&apcState);
	return refAddress;
}
static BOOL RaWMemEx(HANDLE processId, PVOID address, ULONG size, PVOID buffer, UCHAR type)
{
	// 检查进程ID是否为系统进程
	if (processId == 0 || processId == (HANDLE)4)
		return FALSE;
	// 检查地址
	if ((ULONG64)address < 0x10000 || (ULONG64)address > 0x7FFFFFFFFFFF)
		return FALSE;
	// 定义变量
	PEPROCESS tProcess = NULL;
	PMDL mdl = NULL;
	KAPC_STATE apcState = { 0 };
	PVOID newMemory = NULL;
	// 记录状态
	BOOLEAN isDereferenceObject = FALSE;
	BOOLEAN isAttachProcess = FALSE;
	BOOLEAN isAllocareMdl = FALSE;
	BOOLEAN isMapLockedPages = FALSE;
	BOOLEAN isUnmapLockedPages = FALSE;
	__try
	{
		// 获取进程信息
		if (!NT_SUCCESS(PsLookupProcessByProcessId(processId, &tProcess)))
			return FALSE;
		isDereferenceObject = TRUE;
		ObDereferenceObject(tProcess);
		isDereferenceObject = FALSE;
		KeStackAttachProcess(tProcess, &apcState);
		isAttachProcess = TRUE;
		// 创建MDL
		mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);
		if (!mdl)
		{
			KeUnstackDetachProcess(&apcState);
			return FALSE;
		}
		isAllocareMdl = TRUE;
		// 锁定MDL
		MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
		isMapLockedPages = TRUE;
		KeUnstackDetachProcess(&apcState);
		isAttachProcess = FALSE;
		// 映射MDL
		newMemory = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
		if (!newMemory)
		{
			MmUnlockPages(mdl);
			IoFreeMdl(mdl);
			return FALSE;
		}
		isUnmapLockedPages = TRUE;
		// 读写内存
		if (type == 0)
			RtlCopyMemory(buffer, newMemory, size);
		else
			RtlCopyMemory(newMemory, buffer, size);
		// 释放内存
		MmUnmapLockedPages(newMemory, mdl);
		isUnmapLockedPages = FALSE;
		// 解锁MDL
		MmUnlockPages(mdl);
		isMapLockedPages = FALSE;
		// 销毁MDL
		IoFreeMdl(mdl);
		isAllocareMdl = FALSE;
		return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (isUnmapLockedPages)
			MmUnmapLockedPages(newMemory, mdl);
		if (isMapLockedPages)
			MmUnlockPages(mdl);
		if (isAllocareMdl)
			IoFreeMdl(mdl);
		if (isAttachProcess)
			KeUnstackDetachProcess(&apcState);
		if (isDereferenceObject)
			ObDereferenceObject(tProcess);
		return FALSE;
	}
}
BOOL Utils::InjectDll(HANDLE processId, UNICODE_STRING dllPath)
{
	// 检查进程ID是否为系统进程
	if (processId == 0 || processId == (HANDLE)4)
		return FALSE;
	// 判断dll路径大小
	BOOL status = FALSE;
	ULONG size = wcslen(dllPath.Buffer) * sizeof(WCHAR);
	// 申请一块内存用来存放DLL路径
	PVOID newMemory = Utils::NewMem(processId, size);
	if (newMemory == nullptr)
		return FALSE;
	// 写入DLL路径
	//status = Utils::MDLWriteMemory((DWORD)processId, newMemory, size, (BYTE*)dllPath.buffer);
	status = RaWMemEx(processId, newMemory, size, dllPath.Buffer, 1);
	if (status == FALSE)
	{
		Utils::DelMem(processId, newMemory);
		return status;
	}
	//// 获取LoadLibraryW地址
	PVOID loadLibraryAddress = GetProcessAddress(processId, L"kernel32.dll", "LoadLibraryW");
	if (loadLibraryAddress == 0)
	{
		Utils::DelMem(processId, newMemory);
		return FALSE;
	}
	// 创建线程
	status = Utils::CreateThread(processId, loadLibraryAddress, newMemory);
	return status;
	//return TRUE;
}