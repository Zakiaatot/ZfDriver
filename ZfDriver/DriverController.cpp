#include "DriverController.h"
#include "Utils.h"

inline static DWORD CtlCodeGen(DWORD lngFunction)
{
	return (FILE_DEVICE_UNKNOWN * 65536) | (FILE_ANY_ACCESS * 16384) | (lngFunction * 4) | METHOD_BUFFERED;
}

DriverController::DriverController()
	:sysPath_(NULL),
	serviceName_(NULL),
	displayName_(NULL),
	scManager_(NULL),
	service_(NULL),
	driver_(INVALID_HANDLE_VALUE)
{
}

DriverController::~DriverController()
{
}

BOOL DriverController::Install(PCWSTR sysPath, PCWSTR serviceName, PCWSTR displayName)
{
	sysPath_ = sysPath;
	serviceName_ = serviceName;
	displayName_ = displayName;

	do {
		// Open SCManager
		scManager_ = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (scManager_ == NULL)
		{
			Utils::AlertError(L"Open SCManager Error!");
			break;
		}

		// Create Service
		service_ = CreateService
		(
			scManager_,
			serviceName_,
			displayName_,
			SERVICE_ALL_ACCESS,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_IGNORE,
			sysPath_,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);
		if (service_ == NULL)
		{
			DWORD error = GetLastError();
			if (error == ERROR_SERVICE_EXISTS)
			{
				service_ = OpenService(scManager_, serviceName_, SERVICE_ALL_ACCESS);
				if (service_ == NULL)
				{
					CloseServiceHandle(scManager_);
					Utils::AlertError(L"Open Service Error!");
					break;
				}
			}
			else
			{
				CloseServiceHandle(scManager_);
				Utils::AlertError(L"Create Service Error!");
				break;
			}
		}
		return true;
	} while (false);

	return false;
}

BOOL DriverController::Start()
{
	if (!StartService(service_, NULL, NULL))
	{
		Utils::AlertError(L"Start Driver Error!");
		return false;
	}
	return true;
}

BOOL DriverController::Stop()
{
	SERVICE_STATUS ss;
	GetSvcHandle(serviceName_);
	if (!ControlService(service_, SERVICE_CONTROL_STOP, &ss))
	{
		Utils::AlertError(L"Stop Driver Error!");
		return false;
	}
	return true;
}

BOOL DriverController::Uninstall()
{
	GetSvcHandle(serviceName_);
	if (!DeleteService(service_))
	{
		Utils::AlertError(L"Uninstall Driver Error!");
		return false;
	}
	return true;
}

BOOL DriverController::Open(PCWSTR linkName)
{
	if (driver_ != INVALID_HANDLE_VALUE)
		return true;
	driver_ = CreateFile
	(
		linkName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	if (driver_ != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	else
	{
		Utils::AlertError(L"Open Driver Error!");
		return false;
	}
}

BOOL DriverController::IoControl
(
	DWORD ioCode,
	PVOID inBuff,
	DWORD inBuffLen,
	PVOID outBuff,
	DWORD outBuffLen,
	DWORD* realRetBytes
)
{
	DWORD retBytes;
	BOOL res = DeviceIoControl
	(
		driver_,
		CtlCodeGen(ioCode),
		inBuff,
		inBuffLen,
		outBuff,
		outBuffLen,
		&retBytes,
		NULL
	);
	if (realRetBytes)
		*realRetBytes = retBytes;
	return res;
}

BOOL DriverController::GetSvcHandle(PCWSTR serviceName)
{
	serviceName_ = serviceName;
	do
	{
		scManager_ = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (scManager_ == NULL)
		{
			Utils::AlertError(L"Open SCManager Error!");
			break;
		}
		service_ = OpenService(scManager_, serviceName_, SERVICE_ALL_ACCESS);
		if (service_ == NULL)
		{
			CloseServiceHandle(scManager_);
			Utils::AlertError(L"Open Service Error!");
			break;
		}
		return true;
	} while (false);
	return false;
}

BOOL DriverController::Close()
{
	if (driver_ != INVALID_HANDLE_VALUE)
	{
		return CloseHandle(driver_);
	}
	return true;
}