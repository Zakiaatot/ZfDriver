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

typedef DWORD D3DCOLOR;
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_RGB(r,g,b) D3DCOLOR_ARGB(255,r,g,b)

struct IMGCOLOR
{
	float x, y, z, w;
	constexpr IMGCOLOR() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
	constexpr IMGCOLOR(float _x, float _y, float _z, float _w = 1.0) : x(_x), y(_y), z(_z), w(_w) { }
	constexpr IMGCOLOR(int r, int g, int b, int a = 255) : IMGCOLOR((float)r* (1.0f / 255.0f), (float)g* (1.0f / 255.0f), (float)b* (1.0f / 255.0f), (float)a* (1.0f / 255.0f)) {}
};

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
	// GDI Draw
	static BOOL GDIDrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // ��ʼ������
	static BOOL GDIDrawDestroy(); // ��������
	static BOOL GDIDrawFps(); // ����FPS
	static BOOL GDIDrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN COLORREF color, IN INT fontSize = 16); // �����ı�
	static BOOL GDIDrawLine(IN LONG x1, IN LONG y1, IN  LONG x2, IN  LONG y2, IN LONG lineWidth, IN  COLORREF color); // ��������
	static BOOL GDIDrawRect(IN LONG x, IN LONG y, IN LONG width, IN  LONG height, IN  LONG lineWidth, IN COLORREF color); // ���ƾ���
	static BOOL GDIDrawRectFill(IN LONG x, IN LONG y, IN LONG width, IN LONG height, IN COLORREF color); // ������
	static BOOL GDIDrawCircle(IN LONG x, IN LONG y, IN LONG r, IN COLORREF color, IN LONG lineCount, IN LONG lineWidth); // ����ԲȦ
	// D3DX9 Draw
	static BOOL D3DDrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // ��ʼ������
	static BOOL D3DDrawDestroy(); // ��������
	static BOOL D3DDrawFps(IN INT fontSzie); // ����FPS
	static BOOL D3DDrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN D3DCOLOR color, IN INT fontSize = 16); // �����ı�
	static BOOL D3DDrawLine(IN FLOAT x1, IN FLOAT y1, IN  FLOAT x2, IN  FLOAT y2, IN FLOAT lineWidth, IN  D3DCOLOR color); // ��������
	static BOOL D3DDrawRect(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN  FLOAT height, IN  FLOAT lineWidth, IN D3DCOLOR color); // ���ƾ���
	static BOOL D3DDrawRectFill(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN FLOAT height, IN D3DCOLOR color); // ������
	static BOOL D3DDrawCircle(IN FLOAT x, IN FLOAT y, IN FLOAT r, IN D3DCOLOR color, IN LONG lineCount, IN FLOAT lineWidth); // ����ԲȦ
	// IMGUI DX11 Draw
	static BOOL IMGDrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // ��ʼ������
	static BOOL IMGDrawDestroy(); // ��������
	static BOOL IMGDrawFps(); // ����FPS
	static BOOL IMGDrawText(IN FLOAT x, IN FLOAT y, IN LPCWSTR str, IN IMGCOLOR color); // �����ı�
	static BOOL IMGDrawLine(IN FLOAT x1, IN FLOAT y1, IN  FLOAT x2, IN  FLOAT y2, IN FLOAT lineWidth, IN  IMGCOLOR color); // ��������
	static BOOL IMGDrawRect(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN  FLOAT height, IN  FLOAT lineWidth, IN IMGCOLOR color); // ���ƾ���
	static BOOL IMGDrawRectFill(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN FLOAT height, IN IMGCOLOR color); // ������
	static BOOL IMGDrawCircle(IN FLOAT x, IN FLOAT y, IN FLOAT r, IN IMGCOLOR color, IN LONG lineCount, IN FLOAT lineWidth); // ����ԲȦ
	// Utils
	static BOOL ForceDeleteFile(IN PCWSTR filePath); // ǿ��ɾ���ļ�  filePath Ϊ���ַ�·��  ���� L"C:\\123.exe"
	static DWORD64 GetModuleBase(IN DWORD pid, IN PCWSTR moduleName); // ȡ����ģ���ַ
	static BOOL ProcessHide(IN DWORD pid, IN BOOL hide = TRUE); // ���ؽ���  hide==0ʱ�ظ�����  Warning: ���Ҫ�û��ƣ����ȳ�ʼ������ģ�������ؽ���
	static BOOL WindowHide(IN HWND hwnd); // ���ش��� ����ͼ
	static DWORD GetProcessId(IN PCWSTR processName);// ���ݽ������ƻ�ȡID
	static BOOL InjectDll(IN DWORD pid, IN PCWSTR dllPath);// DLLע��
};

#endif // !_ZFDRIVER_H_
