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
} D3DFPS;

class GDI
{
public:
	GDI(HWND hwnd, SUB_FUNC subFunc, INT fontSize);
	~GDI();

public:
	INT GetFps();
	VOID DrawText(LONG x, LONG y, LPCWSTR str, COLORREF color, INT fontSize = 0);
	VOID DrawFps();
public:
	BOOL IsInited() const { return inited_; };

private:
	GDI(const GDI&) = delete;
	const GDI& operator=(const GDI&) = delete;
private:
	static DWORD FuncLoop(LPVOID pGDIObject);
private:
	HWND hwnd_;
	SUB_FUNC subFunc_;
	INT fontSize_;
	LONG width_;
	LONG height_;
	HDC hdc_;
	HDC device_;
	HGDIOBJ object_;
	BOOL inited_;
	D3DFPS fps_;
};
#endif // !_GDI_H_


