#ifndef _UTILS_H_
#define _UTILS_H_

#include <windef.h>

namespace Utils
{
	BOOL MDLReadMemory(IN DWORD pid, IN PVOID address, IN DWORD size, OUT BYTE* res);
}

#endif // !_UTILS_H_
