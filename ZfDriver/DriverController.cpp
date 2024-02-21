#include "DriverController.h"
#include "Utils.h"


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
			Close();
			Stop();
			Uninstall();

			CloseServiceHandle(service_);
			CloseServiceHandle(scManager_);
			//Utils::AlertError(L"Create Service Error! Please Reopen it.");
			break;
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
	if (service_ == NULL)
		GetSvcHandle(serviceName_);
	if (!ControlService(service_, SERVICE_CONTROL_STOP, &ss))
	{
		return false;
	}
	return true;
}

BOOL DriverController::Uninstall()
{
	if (service_ == NULL)
		GetSvcHandle(serviceName_);
	if (!DeleteService(service_))
	{
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
		ioCode,
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
		if (scManager_ == NULL)
			scManager_ = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (scManager_ == NULL)
		{
			Utils::AlertError(L"Open SCManager Error!");
			break;
		}
		if (service_ == NULL)
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