#include "ZfDriver.h"
#include "Utils.h"
#include "Resource.h"
#include "DriverController.h"
#include "IoctlUtils.h"

#define DRIVER_FILE_NAME L"ZfDriver-R0.sys"
#define DRIVER_SERVICE_NAME L"ZfDriver"
#define DRIVER_SYMLINK_NAME L"\\\\.\\ZfDriver"

DriverController gDriverController;
BOOL gIsZfDriverInstalled = FALSE;

BOOL ZfDriver::Install()
{
	if (gIsZfDriverInstalled == TRUE)
		return TRUE;
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
		return FALSE;
	}
	gIsZfDriverInstalled = TRUE;
	return TRUE;
}

VOID ZfDriver::Uninstall()
{
	if (gIsZfDriverInstalled == FALSE)
		return;
	gDriverController.Close();
	gDriverController.Stop();
	gDriverController.Uninstall();
	gIsZfDriverInstalled = FALSE;
}

DWORD ZfDriver::Test(IN DWORD num)
{
	if (gIsZfDriverInstalled == FALSE)
		return num;
	DWORD ret = num;
	gDriverController.IoControl(IOCTL_CODE_TEST, &num, sizeof(DWORD), &ret, sizeof(DWORD), NULL);
	return ret;
}

BOOL ZfDriver::ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, OUT BYTE* data)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_READ trans = { pid,(PVOID)address,size };
	if (!gDriverController.IoControl(IOCTL_CODE_READ, &trans, sizeof(IOCTL_TRANS_READ), data, size, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	BYTE buf[1024] = { 0 };
	IOCTL_TRANS_WRITE* pTrans = (IOCTL_TRANS_WRITE*)buf;
	pTrans->pid = pid;
	pTrans->address = (PVOID)address;
	pTrans->size = size;
	memcpy(&(pTrans->data[0]), data, size);
	if (!gDriverController.IoControl(IOCTL_CODE_WRITE, pTrans, sizeof(IOCTL_TRANS_WRITE) + size, NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::ReadByte(IN DWORD pid, IN DWORD64 address, OUT BYTE* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(BYTE), data);
}

BOOL ZfDriver::ReadShort(IN DWORD pid, IN DWORD64 address, OUT SHORT* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(SHORT), (BYTE*)data);
}

BOOL ZfDriver::ReadInt(IN DWORD pid, IN DWORD64 address, OUT INT* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(INT), (BYTE*)data);
}

BOOL ZfDriver::ReadLong(IN DWORD pid, IN DWORD64 address, OUT LONGLONG* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(LONGLONG), (BYTE*)data);
}

BOOL ZfDriver::ReadFloat(IN DWORD pid, IN DWORD64 address, OUT FLOAT* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(FLOAT), (BYTE*)data);
}

BOOL ZfDriver::ReadDouble(IN DWORD pid, IN DWORD64 address, OUT DOUBLE* data)
{
	return ZfDriver::ReadBytes(pid, address, sizeof(DOUBLE), (BYTE*)data);
}


BOOL ZfDriver::WriteByte(IN DWORD pid, IN DWORD64 address, IN BYTE data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(BYTE), &data);
}

BOOL ZfDriver::WriteShort(IN DWORD pid, IN DWORD64 address, IN SHORT data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(SHORT), (BYTE*)&data);
}

BOOL ZfDriver::WriteInt(IN DWORD pid, IN DWORD64 address, IN INT data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(INT), (BYTE*)&data);
}

BOOL ZfDriver::WriteLong(IN DWORD pid, IN DWORD64 address, IN LONGLONG data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(LONGLONG), (BYTE*)&data);
}

BOOL ZfDriver::WriteFloat(IN DWORD pid, IN DWORD64 address, IN FLOAT data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(FLOAT), (BYTE*)&data);
}

BOOL ZfDriver::WriteDouble(IN DWORD pid, IN DWORD64 address, IN DOUBLE data)
{
	return ZfDriver::WriteBytes(pid, address, sizeof(DOUBLE), (BYTE*)&data);
}

BOOL ZfDriver::ForceDeleteFile(IN PCWSTR filePath)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	if (!gDriverController.IoControl(IOCTL_CODE_FORCE_DELETE_FILE, (PVOID)filePath, (wcslen(filePath) + 1) * sizeof(WCHAR), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

DWORD64 ZfDriver::GetModuleBase(IN DWORD pid, IN PCWSTR moduleName)
{
	if (gIsZfDriverInstalled == FALSE)
		return 0;
	DWORD64 base = 0;
	BYTE buf[1024] = { 0 };
	IOCTL_TRANS_GET_MODULE_BASE* pTrans = (IOCTL_TRANS_GET_MODULE_BASE*)buf;
	pTrans->pid = pid;
	memcpy(pTrans->moduleName, moduleName, (wcslen(moduleName) + 1) * sizeof(WCHAR));
	gDriverController.IoControl
	(
		IOCTL_CODE_GET_MODULE_BASE,
		pTrans,
		sizeof(IOCTL_TRANS_GET_MODULE_BASE) + (wcslen(moduleName) + 1) * sizeof(WCHAR),
		&base,
		sizeof(DWORD64),
		NULL
	);
	return base;
}