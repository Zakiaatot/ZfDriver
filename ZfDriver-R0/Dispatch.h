#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include <ntifs.h>
#include <windef.h>

namespace Dispatch
{
	VOID Register(IN PDRIVER_OBJECT pDriObj);
}

#endif // !_DISPATCH_H_

