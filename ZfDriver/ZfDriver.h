#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

#include <Windows.h>

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
	static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, OUT BYTE* data); // 读字节集: data需自己申请空间且确保空间大于size
	// Write
	static BOOL WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data); // 写字节集: data为写入数据 确保一次写入小于1000字节
};

#endif // !_ZFDRIVER_H_
