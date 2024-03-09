#ifndef _GDI_H_
#define _GDI_H_

#include <Windows.h>
#undef DrawText

typedef VOID(*SUB_FUNC)();
typedef struct {
	INT curFrames;
	FLOAT curTime;
	FLOAT lastTime;
	FLOAT retFps;
} GDIFPS;

class GDI
{
public:
	GDI(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize = 16);
	~GDI();

public:
	INT GetFps();
	VOID DrawFps();
	VOID DrawText(LONG x, LONG y, LPCWSTR str, COLORREF color, INT fontSize = 0);
	VOID DrawLine(LONG x1, LONG y1, LONG x2, LONG y2, LONG lineWidth, COLORREF color);
	VOID DrawRect(LONG x, LONG y, LONG width, LONG height, LONG lineWidth, COLORREF color);
	VOID DrawCircle(LONG x, LONG y, LONG r, COLORREF color, LONG lineCount, LONG lineWidth);
	VOID FillRect(LONG x, LONG y, LONG width, LONG height, COLORREF color);
public:
	HWND GetHwnd() const { return hwnd_; };
	BOOL IsInited() const { return inited_; };

private:
	GDI(const GDI&) = delete;
	const GDI& operator=(const GDI&) = delete;
private:
	static DWORD FuncLoop(LPVOID pGDIObject);
private:
	HWND hwnd_;
	HANDLE drawThreadHandle_;
	SUB_FUNC subFunc_;
	INT fontSize_;
	HBRUSH brush_;
	LONG width_;
	LONG height_;
	HDC hdc_;
	HDC device_;
	HGDIOBJ object_;
	BOOL inited_;
	GDIFPS fps_;
};
#endif // !_GDI_H_


