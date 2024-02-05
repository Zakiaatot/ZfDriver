#include "ZfDriver.h"
#include "Utils.h"
#include "Resource.h"
#include "DriverController.h"

#define DRIVER_FILE_NAME L"ZfDriver-R0.sys"
#define DRIVER_SERVICE_NAME L"ZfDriver"
#define DRIVER_SYMLINK_NAME L"\\\\.\\ZfDriver"

DriverController gDriverController;
bool gIsZfDriverInstalled = false;

bool ZfDriver::Install()
{
	if (gIsZfDriverInstalled == true)
		return true;
	wchar_t sysPath[MAX_PATH] = { 0 };
	Utils::GetAppPath(sysPath);
	wcscat_s(sysPath, DRIVER_FILE_NAME);
	if (
		!Utils::ReleaseResource(IDR_SYS1, L"SYS", DRIVER_FILE_NAME) ||
		!gDriverController.Install(sysPath, DRIVER_SERVICE_NAME, DRIVER_SERVICE_NAME) ||
		!gDriverController.Start() ||
		!gDriverController.Open(DRIVER_SYMLINK_NAME)
		)
	{
		return false;
	}
	gIsZfDriverInstalled = true;
	return true;
}

void ZfDriver::Uninstall()
{
	if (gIsZfDriverInstalled == false)
		return;
	gDriverController.Close();
	gDriverController.Stop();
	gDriverController.Uninstall();
	gIsZfDriverInstalled = false;
}