#ifndef _DRIVER_CONTROLLER_H_
#define _DRIVER_CONTROLLER_H_

#include <Windows.h>

class DriverController
{
public:
	PCWSTR sysPath_;
	PCWSTR serviceName_;
	PCWSTR displayName_;
	HANDLE driver_;
	SC_HANDLE scManager_;
	SC_HANDLE service_;
public:
	DriverController();
	~DriverController();
	BOOL Install(PCWSTR sysPath, PCWSTR serviceName, PCWSTR displayName);
	BOOL Uninstall();
	BOOL Start();
	BOOL Stop();
	BOOL Open(PCWSTR linkName);
	BOOL Close();
	BOOL IoControl
	(
		DWORD ioCode,
		PVOID inBuff,
		DWORD inBuffLen,
		PVOID outBuff,
		DWORD outBuffLen,
		DWORD* realRetBytes
	);
private:
	BOOL GetSvcHandle(PCWSTR serviceName);
};

#endif // !_DRIVER_CONTROLLER_H_


