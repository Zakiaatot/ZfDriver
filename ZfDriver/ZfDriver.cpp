#define DLLEXPORT

#include "ZfDriver.h"
#include "Utils.h"
#include "Resource.h"
#include "DriverController.h"
#include "IoctlUtils.h"
#include "GDI.h"

#define DRIVER_FILE_NAME L"ZfDriver-R0.sys"
#define DRIVER_SERVICE_NAME L"ZfDriver"
#define DRIVER_SYMLINK_NAME L"\\\\.\\ZfDriver"

DriverController gDriverController;
BOOL gIsZfDriverInstalled = FALSE;
GDI* gPGDIObject = NULL;

BOOL ZfDriver::Install()
{
	if (gIsZfDriverInstalled == TRUE)
		return TRUE;
	WCHAR sysPath[MAX_PATH] = { 0 };
	Utils::GetAppPath(sysPath);
	wcscat_s(sysPath, DRIVER_FILE_NAME);
	if (!Utils::ReleaseResource(IDR_SYS1, L"SYS", DRIVER_FILE_NAME))
		return FALSE;
	int retryCount = 3;
	while (retryCount > 0 && !gDriverController.Install(sysPath, DRIVER_SERVICE_NAME, DRIVER_SERVICE_NAME))
	{
		retryCount--;
	}
	if (retryCount == 0)
	{
		return FALSE;
	}
	if (
		!gDriverController.Start() ||
		!gDriverController.Open(DRIVER_SYMLINK_NAME)
		)
	{
		return FALSE;
	}
	gIsZfDriverInstalled = TRUE;
	ZfDriver::ForceDeleteFile(sysPath);
	VOID(*pAutoUninstallFunc)() = ZfDriver::Uninstall;
	atexit(pAutoUninstallFunc);
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
	if (!gDriverController.IoControl(IOCTL_CODE_FORCE_DELETE_FILE, (PVOID)filePath, (DWORD)(wcslen(filePath) + 1) * sizeof(WCHAR), NULL, 0, NULL))
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
		(DWORD)sizeof(IOCTL_TRANS_GET_MODULE_BASE) + (DWORD)(wcslen(moduleName) + 1) * sizeof(WCHAR),
		&base,
		sizeof(DWORD64),
		NULL
	);
	return base;
}

BOOL ZfDriver::KeyDown(IN USHORT keyCode)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_KBD  trans = { 0 };
	trans.MakeCode = (USHORT)MapVirtualKey(keyCode, 0);
	if (!gDriverController.IoControl(IOCTL_CODE_KBD, (PVOID)&trans, sizeof(IOCTL_TRANS_KBD), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::keyUp(IN USHORT keyCode)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_KBD  trans = { 0 };
	trans.MakeCode = (USHORT)MapVirtualKey(keyCode, 0);
	trans.Flags = 1;
	if (!gDriverController.IoControl(IOCTL_CODE_KBD, (PVOID)&trans, sizeof(IOCTL_TRANS_KBD), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseLeftButtonDown()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0001;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseLeftButtonUp()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0002;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseRightButtonDown()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0004;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseRightButtonUp()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0008;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseMiddleButtonDown()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0010;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseMiddleButtonUp()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.ButtonFlags = 0x0020;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseMoveRelative(IN LONG dx, IN LONG dy)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.Flags = 0;
	trans.LastX = dx;
	trans.LastY = dy;
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::MouseMoveAbsolute(IN LONG dx, IN LONG dy)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_MOU  trans = { 0 };
	trans.Flags = 1;
	trans.LastX = dx * 0xffff / GetSystemMetrics(SM_CXSCREEN);
	trans.LastY = dy * 0xffff / GetSystemMetrics(SM_CYSCREEN);
	if (!gDriverController.IoControl(IOCTL_CODE_MOU, (PVOID)&trans, sizeof(IOCTL_TRANS_MOU), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::DrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	if (gPGDIObject)
	{
		delete gPGDIObject;
		gPGDIObject = NULL;
	}
	INT width = GetSystemMetrics(SM_CXSCREEN);
	INT height = GetSystemMetrics(SM_CYSCREEN);
	gPGDIObject = new GDI(width, height, drawLoop, fontSize);
	HWND hwnd = gPGDIObject->GetHwnd();
	if (hwnd)
	{
		ZfDriver::WindowHide(hwnd);
	}
	return gPGDIObject->IsInited();
}

BOOL ZfDriver::DrawDestroy()
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	if (gPGDIObject)
	{
		delete gPGDIObject;
		gPGDIObject = NULL;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL ZfDriver::DrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN COLORREF color, IN INT fontSize)
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->DrawText(x, y, str, color, fontSize);
	return TRUE;
}

BOOL ZfDriver::DrawFps()
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->DrawFps();
	return TRUE;
}

BOOL ZfDriver::DrawLine(IN LONG x1, IN LONG y1, IN LONG x2, IN LONG y2, IN LONG lineWidth, IN COLORREF color)
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->DrawLine(x1, y1, x2, y2, lineWidth, color);
	return TRUE;
}

BOOL ZfDriver::DrawRect(IN LONG x, IN LONG y, IN LONG width, IN LONG height, IN LONG lineWidth, IN COLORREF color)
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->DrawRect(x, y, width, height, lineWidth, color);
	return TRUE;
}

BOOL ZfDriver::DrawCircle(IN LONG x, IN LONG y, IN LONG r, IN COLORREF color, IN LONG lineCount, IN LONG lineWidth)
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->DrawCircle(x, y, r, color, lineCount, lineWidth);
	return TRUE;
}

BOOL ZfDriver::DrawRectFill(IN LONG x, IN LONG y, IN LONG width, IN LONG height, IN COLORREF color)
{
	if (gPGDIObject == NULL)
		return FALSE;
	gPGDIObject->FillRect(x, y, width, height, color);
	return TRUE;
}

BOOL ZfDriver::ProcessHide(IN DWORD pid, IN BOOL hide)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	IOCTL_TRANS_PROCESS_HIDE  trans = { 0 };
	trans.pid = pid;
	trans.hide = hide;
	if (!gDriverController.IoControl(IOCTL_CODE_PROCESS_HIDE, (PVOID)&trans, sizeof(IOCTL_TRANS_PROCESS_HIDE), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

BOOL ZfDriver::WindowHide(IN HWND hwnd)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	if (!gDriverController.IoControl(IOCTL_CODE_WINDOW_HIDE, (PVOID)&hwnd, sizeof(HWND), NULL, 0, NULL))
		return FALSE;
	return TRUE;
}

DWORD ZfDriver::GetProcessId(IN PCWSTR processName)
{
	if (gIsZfDriverInstalled == FALSE)
		return 0;
	DWORD id = 0;
	if (!gDriverController.IoControl(IOCTL_CODE_GET_PROCESS_ID, (PVOID)processName, (DWORD)(wcslen(processName) + 1) * sizeof(WCHAR), &id, sizeof(DWORD), NULL))
		return 0;
	return id;
}

BOOL ZfDriver::InjectDll(IN DWORD pid, IN PCWSTR dllPath)
{
	if (gIsZfDriverInstalled == FALSE)
		return FALSE;
	BYTE buf[1024] = { 0 };
	IOCTL_TRANS_INJECT_DLL* pTrans = (IOCTL_TRANS_INJECT_DLL*)buf;
	pTrans->pid = pid;
	memcpy(pTrans->dllPath, dllPath, (wcslen(dllPath) + 1) * sizeof(WCHAR));
	if (
		!gDriverController.IoControl
		(
			IOCTL_CODE_INJECT_DLL,
			pTrans,
			(DWORD)sizeof(IOCTL_CODE_INJECT_DLL) + (DWORD)(wcslen(dllPath) + 1) * sizeof(WCHAR),
			NULL,
			0,
			NULL
		)
		)
	{
		return FALSE;
	}
	return TRUE;
}