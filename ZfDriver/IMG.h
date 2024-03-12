#ifndef _IMG_H_
#define _IMG_H_

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#undef DrawText

typedef VOID(*SUB_FUNC)();

class IMG
{
public:
	IMG(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize = 16);
	~IMG();

public:
	INT GetFps();
	VOID DrawFps();
	VOID DrawText(FLOAT x, FLOAT y, LPCWSTR str, ImColor color);
	VOID DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT lineWidth, ImColor color);
	VOID DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT lineWidth, ImColor color);
	VOID DrawCircle(FLOAT x, FLOAT y, FLOAT r, ImColor color, LONG lineCount, FLOAT lineWidth);
	VOID FillRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, ImColor color);
	HWND GetHwnd() const { return hwnd_; };
	BOOL IsInited() const { return inited_; };

private:
	IMG(const IMG&) = delete;
	IMG& operator=(const IMG&) = delete;
	IMG(const IMG&&) = delete;
	IMG& operator=(const IMG&&) = delete;
	static DWORD FuncLoop(LPVOID pIMGObject);
	SUB_FUNC subFunc_;
	LONG width_;
	LONG height_;
	BOOL inited_;
	WNDCLASSEX window_;
	HWND hwnd_;
	INT fontSize_;
	HANDLE drawThreadHandle_;
};

#endif //!_IMG_H_
