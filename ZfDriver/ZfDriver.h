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
	static BOOL ReadByte(IN DWORD pid, IN DWORD64 address, OUT BYTE* data); // 读字节
	static BOOL ReadShort(IN DWORD pid, IN DWORD64 address, OUT SHORT* data); // 读短整数
	static BOOL ReadInt(IN DWORD pid, IN DWORD64 address, OUT INT* data); // 读整数
	static BOOL ReadLong(IN DWORD pid, IN DWORD64 address, OUT LONGLONG* data); // 读长整数
	static BOOL ReadFloat(IN DWORD pid, IN DWORD64 address, OUT FLOAT* data); // 读小数
	static BOOL ReadDouble(IN DWORD pid, IN DWORD64 address, OUT DOUBLE* data); // 读双精度小数
	// Write
	static BOOL WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data); // 写字节集: data为写入数据 确保一次写入小于1000字节
	static BOOL WriteByte(IN DWORD pid, IN DWORD64 address, IN BYTE data); // 写字节
	static BOOL WriteShort(IN DWORD pid, IN DWORD64 address, IN SHORT data); // 写短整数
	static BOOL WriteInt(IN DWORD pid, IN DWORD64 address, IN INT data); // 写整数
	static BOOL WriteLong(IN DWORD pid, IN DWORD64 address, IN LONGLONG data); // 写长整数
	static BOOL WriteFloat(IN DWORD pid, IN DWORD64 address, IN FLOAT data); // 写小数
	static BOOL WriteDouble(IN DWORD pid, IN DWORD64 address, IN DOUBLE data); // 写双精度小数
	// Utils
	static BOOL ForceDeleteFile(IN PCWSTR filePath); // 强制删除文件  filePath 为宽字符路径  例如 L"C:\\123.exe"
	static DWORD64 GetModuleBase(IN DWORD pid, IN PCWSTR moduleName); // 取进程模块基址
};

#endif // !_ZFDRIVER_H_
