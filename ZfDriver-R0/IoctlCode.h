#ifndef _IOCTL_CODE_H_
#define _IOCTL_CODE_H_

#include <ntifs.h>

#define IOCTL_TEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif // !_IOCTL_CODE_H_