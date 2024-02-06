#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

#include <Windows.h>

#define IN // ��ʾ���
#define OUT // ��ʾ����
#define INOUT // ��ʾ�����

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
	static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, INOUT BYTE* res); // ���ֽڼ�: res���Լ�������ȷ���ռ����size
	// Write
};

#endif // !_ZFDRIVER_H_
