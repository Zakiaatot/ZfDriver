#include <sstream>
#include <iostream>
#include "GDI.h"

#pragma comment (lib,"Winmm.lib")

static VOID HandleEvent()
{
	if (!GetInputState())
		return;
	MSG message;
	while (::GetMessage(&message, NULL, 0, 0)) {
		::TranslateMessage(&message);
		::DispatchMessage(&message);
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

GDI::GDI(HWND hwnd, SUB_FUNC subFunc, INT fontSize)
	:hwnd_(hwnd),
	subFunc_(subFunc),
	fontSize_(fontSize),
	width_(0),
	height_(0),
	hdc_(0),
	device_(0),
	object_(0),
	inited_(FALSE),
	fps_({ 0 })
{
	hdc_ = GetDC(hwnd);
	SetLayeredWindowAttributes(hwnd, GetBkColor(hdc_), 255, 3);
	CreateThread(0, 0, GDI::FuncLoop, this, 0, 0);
	inited_ = TRUE;
}

GDI::~GDI()
{
	if (inited_)
		ShowWindowAsync(hwnd_, 0);
}

DWORD GDI::FuncLoop(LPVOID pGDIObject)
{
	GDI* obj = (GDI*)pGDIObject;
	DOUBLE scale = GetScreenScale();
	while (IsWindow(obj->hwnd_))
	{
		RECT rc = { 0 };
		GetWindowRect(obj->hwnd_, &rc);
		obj->width_ = (rc.right - rc.left) * scale;
		obj->height_ = (rc.bottom - rc.top) * scale;

		std::cout << "Func Loop" << std::endl;
		Sleep(1);
		HandleEvent();
		std::cout << "Width: " << obj->width_ << " Height: " << obj->height_ << std::endl;
		HBITMAP bmp = CreateCompatibleBitmap(obj->hdc_, obj->width_, obj->height_);
		obj->device_ = CreateCompatibleDC(obj->hdc_);
		SelectObject(obj->device_, bmp);
		BitBlt(obj->device_, 0, 0, obj->width_, obj->height_, 0, 0, 0, WHITENESS);
		if (obj->device_ == 0)
			break;
		if (obj->subFunc_ != NULL)
		{
			std::cout << "Sub Func" << std::endl;
			obj->subFunc_();
		}
		BitBlt(obj->hdc_, 0, 0, obj->width_, obj->height_, obj->device_, 0, 0, SRCCOPY);
		DeleteDC(obj->device_);
		DeleteObject(bmp);
	}
	std::cout << "Func Loop Exited." << std::endl;
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


VOID GDI::DrawFps()
{
	std::wstringstream wss;
	wss << L"GDI FPS: " << GDI::GetFps();
	GDI::DrawText(10, 10, wss.str().c_str(), RGB(0, 255, 0), 24);
}