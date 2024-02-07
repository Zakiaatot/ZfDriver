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
	static DWORD Test(IN DWORD num); // ����: ������� ���� num+1
	// Read
	static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, OUT BYTE* data); // ���ֽڼ�: data���Լ�����ռ���ȷ���ռ����size
	// Write
	static BOOL WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data); // д�ֽڼ�: dataΪд������ ȷ��һ��д��С��1000�ֽ�
};

#endif // !_ZFDRIVER_H_
