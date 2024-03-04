#ifndef _UTILS_H_
#define _UTILS_H_

#include <windef.h>

namespace Utils
{
	BOOL MDLReadMemory(IN DWORD pid, IN PVOID address, IN DWORD size, OUT BYTE* data);
	BOOL MDLWriteMemory(IN DWORD pid, IN PVOID address, IN DWORD size, IN BYTE* data);
	BOOL ForceDeleteFile(IN UNICODE_STRING pwzFileName);
	DWORD64 GetModuleBaseWow64(IN PEPROCESS pEProcess, IN UNICODE_STRING moduleName);
	DWORD64 GetModuleBase64(IN PEPROCESS pEProcess, IN UNICODE_STRING moduleName);
	BOOL ProcessHide(IN DWORD pid, IN BOOL hide = TRUE);
	BOOL WindowHide(IN HWND hwnd);
	DWORD GetProcessId(IN UNICODE_STRING processName);
	PVOID NewMem(IN HANDLE processId, IN ULONG size);
	BOOL DelMem(IN HANDLE processId, IN PVOID address);
	BOOL CreateThread(IN HANDLE processId, IN PVOID startAddress, IN PVOID parmaAddress);
	BOOL InjectDll(IN HANDLE processId, IN UNICODE_STRING dllPath);
}

#endif // !_UTILS_H_
