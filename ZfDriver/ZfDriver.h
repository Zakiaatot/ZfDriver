#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

#include <Windows.h>

#define IN // 表示入参
#define OUT // 表示出参
#define INOUT // 表示出入参

class ZfDriver
{
private:
	ZfDriver() = default;
	~ZfDriver() = default;
public:
	static BOOL Install();
	static VOID Uninstall();
	static DWORD Test(IN DWORD num); // 测试: 如果正常 返回 num+1
	// Read
	static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, INOUT BYTE* res); // 读字节集: res需自己申请且确保空间大于size
	// Write
};

#endif // !_ZFDRIVER_H_
