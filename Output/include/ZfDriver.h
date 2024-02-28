#ifndef _ZFDRIVER_H_
#define _ZFDRIVER_H_

#ifdef DLLEXPORT
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


#include <Windows.h>
#undef DrawText

typedef VOID(*DRAW_LOOP)();

class DLL ZfDriver
{
private:
	ZfDriver() = default;
	~ZfDriver() = default;
public:
	static BOOL Install(); // ������װ
	static VOID Uninstall(); // ����ж��
	static DWORD Test(IN DWORD num); // ��������: ������� ���� num+1
	// Read
	static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, OUT BYTE* data); // ���ֽڼ�: data���Լ�����ռ���ȷ���ռ����size
	static BOOL ReadByte(IN DWORD pid, IN DWORD64 address, OUT BYTE* data); // ���ֽ�
	static BOOL ReadShort(IN DWORD pid, IN DWORD64 address, OUT SHORT* data); // ��������
	static BOOL ReadInt(IN DWORD pid, IN DWORD64 address, OUT INT* data); // ������
	static BOOL ReadLong(IN DWORD pid, IN DWORD64 address, OUT LONGLONG* data); // ��������
	static BOOL ReadFloat(IN DWORD pid, IN DWORD64 address, OUT FLOAT* data); // ��С��
	static BOOL ReadDouble(IN DWORD pid, IN DWORD64 address, OUT DOUBLE* data); // ��˫����С��
	// Write
	static BOOL WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data); // д�ֽڼ�: dataΪд������ ȷ��һ��д��С��1000�ֽ�
	static BOOL WriteByte(IN DWORD pid, IN DWORD64 address, IN BYTE data); // д�ֽ�
	static BOOL WriteShort(IN DWORD pid, IN DWORD64 address, IN SHORT data); // д������
	static BOOL WriteInt(IN DWORD pid, IN DWORD64 address, IN INT data); // д����
	static BOOL WriteLong(IN DWORD pid, IN DWORD64 address, IN LONGLONG data); // д������
	static BOOL WriteFloat(IN DWORD pid, IN DWORD64 address, IN FLOAT data); // дС��
	static BOOL WriteDouble(IN DWORD pid, IN DWORD64 address, IN DOUBLE data); // д˫����С��
	// Keyboard and Mouse
	static BOOL KeyDown(IN USHORT keyCode); // ���̰���
	static BOOL keyUp(IN USHORT keyCode); // ���̵���
	static BOOL MouseLeftButtonDown(); // ����������
	static BOOL MouseLeftButtonUp(); // ����������
	static BOOL MouseRightButtonDown(); // ����Ҽ�����
	static BOOL MouseRightButtonUp(); // ����Ҽ�����
	static BOOL MouseMiddleButtonDown(); // �����ְ���
	static BOOL MouseMiddleButtonUp(); // �����ֵ���
	static BOOL MouseMoveRelative(IN LONG dx, IN LONG dy); // �������ƶ�
	static BOOL MouseMoveAbsolute(IN LONG dx, IN LONG dy); // �������ƶ�
	// Draw
	static BOOL DrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // ��ʼ������
	static BOOL DrawFps(); // ����FPS
	static BOOL DrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN COLORREF color, IN INT fontSize = 16); // �����ı�
	static BOOL DrawLine(IN LONG x1, IN LONG y1, IN  LONG x2, IN  LONG y2, IN LONG lineWidth, IN  COLORREF color); // ��������
	static BOOL DrawRect(IN LONG x, IN LONG y, IN LONG width, IN  LONG height, IN  LONG lineWidth, IN COLORREF color); // ���ƾ���
	static BOOL DrawRectFill(IN LONG x, IN LONG y, IN LONG width, IN LONG height, IN COLORREF color); // ������
	static BOOL DrawCircle(IN LONG x, IN LONG y, IN LONG r, IN COLORREF color, IN LONG lineCount, IN LONG lineWidth); // ����ԲȦ
	// Utils
	static BOOL ForceDeleteFile(IN PCWSTR filePath); // ǿ��ɾ���ļ�  filePath Ϊ���ַ�·��  ���� L"C:\\123.exe"
	static DWORD64 GetModuleBase(IN DWORD pid, IN PCWSTR moduleName); // ȡ����ģ���ַ
	static BOOL ProcessHide(IN DWORD pid); // ���ؽ���
	static BOOL WindowHide(IN HWND hwnd); // ���ش��� ����ͼ
	static DWORD GetProcessId(IN PCWSTR processName);// ���ݽ������ƻ�ȡID
	static BOOL InjectDll(IN DWORD pid, IN PCWSTR dllPath);// DLLע��
};

#endif // !_ZFDRIVER_H_
