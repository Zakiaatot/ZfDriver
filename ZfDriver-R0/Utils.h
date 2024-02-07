#ifndef _UTILS_H_
#define _UTILS_H_

#include <windef.h>

namespace Utils
{
	BOOL MDLReadMemory(IN DWORD pid, IN PVOID address, IN DWORD size, OUT BYTE* data);
	BOOL MDLWriteMemory(IN DWORD pid, IN PVOID address, IN DWORD size, IN BYTE* data);
	BOOL ForceDeleteFile(IN UNICODE_STRING pwzFileName);
}

#endif // !_UTILS_H_
