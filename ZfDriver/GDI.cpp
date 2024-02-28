#include <sstream>
#include <cmath>
#include "GDI.h"

#pragma comment (lib,"Winmm.lib")

#define PI acos(-1)

static LRESULT CALLBACK WindowProc
(
	IN HWND hwnd,
	IN UINT uMsg,
	IN WPARAM wParam,
	IN LPARAM lParam
)
{
	if (uMsg == 2)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static VOID HandleEvent()
{
	MSG msg = { 0 };
	while (PeekMessageW(&msg, 0, 0, 0, 1) != 0)
	{
		DispatchMessageW(&msg);
		TranslateMessage(&msg);
	}
}

static DOUBLE GetScreenScale() {
	INT screenW = ::GetSystemMetrics(SM_CXSCREEN);
	INT screenH = ::GetSystemMetrics(SM_CYSCREEN);
	HWND hwnd = ::GetDesktopWindow();
	HDC hdc = ::GetDC(hwnd);
	INT width = ::GetDeviceCaps(hdc, DESKTOPHORZRES);
	INT height = ::GetDeviceCaps(hdc, DESKTOPVERTRES);
	DOUBLE scale = (DOUBLE)width / screenW;
	return scale;
}

GDI::GDI(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize)
	:hwnd_(0),
	drawThreadHandle_(0),
	subFunc_(subFunc),
	fontSize_(fontSize),
	brush_(0),
	width_(width),
	height_(height),
	hdc_(0),
	device_(0),
	object_(0),
	inited_(FALSE),
	fps_({ 0 })
{
	brush_ = CreateSolidBrush(TRANSPARENT);
	WNDCLASSEX window = { 0 };
	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	window.hIcon = 0;
	window.hCursor = 0;
	window.hInstance = (HINSTANCE)123456789;
	window.hbrBackground = brush_;
	window.hIconSm = 0;
	window.lpfnWndProc = WindowProc;
	window.lpszClassName = L"Microsoft Edge";
	window.lpszMenuName = L"ZfDriver GDI";
	RegisterClassEx(&window);
	DWORD dwStyle = WS_CAPTION | WS_POPUP | WS_THICKFRAME;

	hwnd_ = CreateWindowExW
	(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		window.lpszClassName,
		window.lpszMenuName,
		WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_TABSTOP | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		0,
		0,
		width_,
		height_,
		0,
		0,
		window.hInstance,
		0
	);

	dwStyle = GetWindowLongA(hwnd_, GWL_EXSTYLE);
	dwStyle = dwStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED;
	SetWindowLongA(hwnd_, GWL_EXSTYLE, dwStyle);


	hdc_ = GetDC(hwnd_);
	SetLayeredWindowAttributes(hwnd_, GetBkColor(hdc_), 255, 3);
	ShowWindow(hwnd_, 10);
	UpdateWindow(hwnd_);
	DeleteObject(brush_);
	drawThreadHandle_ = CreateThread(0, 0, GDI::FuncLoop, this, 0, 0);
	inited_ = TRUE;
}

GDI::~GDI()
{
	if (inited_)
	{
		ShowWindowAsync(hwnd_, 0);
		if (drawThreadHandle_)
			CloseHandle(drawThreadHandle_);
	}
}

DWORD GDI::FuncLoop(LPVOID pGDIObject)
{
	GDI* obj = (GDI*)pGDIObject;
	DOUBLE scale = GetScreenScale();
	timeBeginPeriod(1);
	while (obj->subFunc_)
	{
		Sleep(5);
		HandleEvent();
		HBITMAP bmp = CreateCompatibleBitmap(obj->hdc_, obj->width_, obj->height_);
		obj->device_ = CreateCompatibleDC(obj->hdc_);
		SelectObject(obj->device_, bmp);
		BitBlt(obj->device_, 0, 0, obj->width_, obj->height_, 0, 0, 0, WHITENESS);

		obj->subFunc_();

		BitBlt(obj->hdc_, 0, 0, obj->width_, obj->height_, obj->device_, 0, 0, SRCCOPY);
		DeleteDC(obj->device_);
		DeleteObject(bmp);
	}
	timeEndPeriod(1);
	return ReleaseDC(obj->hwnd_, obj->hdc_);
}

INT GDI::GetFps()
{
	fps_.curFrames++;
	fps_.curTime = timeGetTime() * 0.001;
	if (fps_.curTime - fps_.lastTime > 1)
	{
		fps_.retFps = fps_.curFrames / (fps_.curTime - fps_.lastTime);
		fps_.lastTime = fps_.curTime;
		fps_.curFrames = 0;
	}
	return fps_.retFps;
}

VOID GDI::DrawText(LONG x, LONG y, LPCWSTR str, COLORREF color, INT fontSize)
{
	if (fontSize == 0)
		fontSize = fontSize_;
	HFONT hFont = CreateFont(fontSize, 0, 0, 0, 0, 0, 0, 0, 134, 0, 0, 0, 0, L"Arial");
	SetTextColor(device_, color);
	SetBkColor(device_, RGB(0, 0, 0));
	SetBkMode(device_, 1);
	object_ = SelectObject(device_, hFont);
	TextOutW(device_, x, y, str, wcslen(str));
	SelectObject(device_, object_);
	DeleteObject(hFont);
}

VOID GDI::DrawLine(LONG x1, LONG y1, LONG x2, LONG y2, LONG lineWidth, COLORREF color)
{
	HPEN hPen = CreatePen(PS_SOLID, lineWidth, color);
	HGDIOBJ object = SelectObject(device_, hPen);
	MoveToEx(device_, x1, y1, 0);
	LineTo(device_, x2, y2);
	SelectObject(device_, object);
	DeleteObject(hPen);
}

VOID GDI::DrawRect(LONG x, LONG y, LONG width, LONG height, LONG lineWidth, COLORREF color)
{
	RECT rc = { x,y,x + width,y + height };
	HPEN hPen = CreatePen(PS_SOLID, lineWidth, color);
	HGDIOBJ object = SelectObject(device_, hPen);
	Rectangle(device_, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(device_, object);
	DeleteObject(hPen);
}

VOID GDI::DrawCircle(LONG x, LONG y, LONG r, COLORREF color, LONG lineCount, LONG lineWidth)
{
	FLOAT step = PI * 2 / lineCount;
	FLOAT size = r;
	FLOAT tmp = 0;
	FLOAT x1, y1, x2, y2;
	for (; lineCount > 0; lineCount--)
	{
		x1 = size * cos(tmp) + x;
		y1 = size * sin(tmp) + y;
		x2 = size * cos(tmp + step) + x;
		y2 = size * sin(tmp + step) + y;
		tmp += step;
		GDI::DrawLine(x1, y1, x2, y2, lineWidth, color);
	}
}

VOID GDI::FillRect(LONG x, LONG y, LONG width, LONG height, COLORREF color)
{
	RECT rc = { x,y,x + width,y + height };
	HBRUSH hBrush = CreateSolidBrush(color);
	HGDIOBJ object = SelectObject(device_, hBrush);
	Rectangle(device_, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(device_, object);
	DeleteObject(hBrush);
}


VOID GDI::DrawFps()
{
	std::wstringstream wss;
	wss << L"FPS: " << GDI::GetFps();
	GDI::DrawText(20, 20, wss.str().c_str(), RGB(0, 255, 0), 24);
}