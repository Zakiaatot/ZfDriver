#ifndef _D3D_H_
#define _D3D_H_

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#undef DrawText

typedef VOID(*SUB_FUNC)();
typedef struct {
	INT curFrames;
	FLOAT curTime;
	FLOAT lastTime;
	FLOAT retFps;
} D3DFPS;

class D3D
{
public:
	D3D(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize = 16);
	~D3D();

public:
	INT GetFps();
	VOID DrawFps(INT fontSize = 16);
	VOID DrawText(LONG x, LONG y, LPCWSTR str, D3DCOLOR color, INT fontSize = 16);
	VOID DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT lineWidth, D3DCOLOR color);
	VOID DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT lineWidth, D3DCOLOR color);
	VOID DrawCircle(FLOAT x, FLOAT y, FLOAT r, D3DCOLOR color, LONG lineCount, FLOAT lineWidth);
	VOID FillRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, D3DCOLOR color);
public:
	HWND GetHwnd() const { return hwnd_; };
	BOOL IsInited() const { return inited_; };

private:
	D3D(const D3D&) = delete;
	D3D& operator=(const D3D&) = delete;
	D3D(const D3D&&) = delete;
	D3D& operator=(const D3D&&) = delete;
private:
	static DWORD FuncLoop(LPVOID pD3DObject);
	VOID ReCreateFont(INT fontSize);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static VOID Reset();
private:
	SUB_FUNC subFunc_;
	LONG width_;
	LONG height_;
	BOOL inited_;
	WNDCLASSEX window_;
	static HWND hwnd_;
	static D3DFPS fps_;
	static LPDIRECT3D9 pD3d_;
	static LPDIRECT3DDEVICE9 pD3dDevice_;
	static D3DPRESENT_PARAMETERS    d3dpp_;
	static ID3DXLine* pLine_;
	static ID3DXFont* pFont_;
	static INT fontSize_;
	static HANDLE drawThreadHandle_;
};

#endif // !_D3D_H_


