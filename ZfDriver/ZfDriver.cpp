#include "ZfDriver.h"
#include "Utils.h"
#include "Resource.h"

#define DRIVER_FILE_NAME L"ZfDriver-R0.sys"


ZfDriver::ZfDriver()
	:installed_(false)
{
}

ZfDriver::~ZfDriver()
{
}

bool ZfDriver::Install()
{
	do
	{
		if (!Utils::ReleaseResource(IDR_SYS1, L"SYS", DRIVER_FILE_NAME))
			break;
		// TODO
		// Install Driver File
	} while (false);
	return installed_;
}