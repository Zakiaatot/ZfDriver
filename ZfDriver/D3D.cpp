#include <sstream>
#include <cmath>
#include "D3D.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")

#define PI acos(-1)

HWND D3D::hwnd_ = 0;
LPDIRECT3D9 D3D::pD3d_ = 0;
LPDIRECT3DDEVICE9 D3D::pD3dDevice_ = 0;
D3DPRESENT_PARAMETERS    D3D::d3dpp_ = { 0 };
ID3DXLine* D3D::pLine_ = NULL;
ID3DXFont* D3D::pFont_ = NULL;
INT D3D::fontSize_ = 16;
HANDLE D3D::drawThreadHandle_ = 0;
D3DFPS D3D::fps_ = { 0 };
D3D* g = 0;
LONG gW = 0;
INT gH = 0;

static INT GetScreenResolution() {
	// 获取主显示器的句柄
	HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);

	// 获取显示器信息
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &monitorInfo);

	// 获取分辨率
	return monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
}


VOID D3D::Reset()
{
	if (pD3d_)
	{
		if (pFont_ != NULL)
		{
			pFont_->Release();
			pFont_ = NULL;
		}

		if (pLine_ != NULL)
		{
			pLine_->Release();
			pLine_ = NULL;
		}

		if (pD3dDevice_ != NULL)
		{
			pD3dDevice_->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
			pD3dDevice_->Release();
			pD3dDevice_ = NULL;
		}

		if (pD3d_ != NULL)
		{
			pD3d_->Release();
			pD3d_ = NULL;
		}
		if (drawThreadHandle_)
		{
			CloseHandle(drawThreadHandle_);
		}
		fps_ = { 0 };
		pD3d_ = Direct3DCreate9(D3D_SDK_VERSION);
		d3dpp_.Windowed = TRUE;
		d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp_.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp_.EnableAutoDepthStencil = TRUE;
		d3dpp_.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		pD3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd_, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp_, &pD3dDevice_);
		D3DXCreateLine(pD3dDevice_, &pLine_);
		D3DXCreateFontW(pD3dDevice_, fontSize_, 0, FW_DONTCARE, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Vernada", &pFont_);
		drawThreadHandle_ = CreateThread(0, 0, D3D::FuncLoop, g, 0, 0);
	}
}

LRESULT CALLBACK D3D::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DISPLAYCHANGE | WM_DPICHANGED:
		Reset();
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}


D3D::D3D(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize)
	:
	subFunc_(subFunc),
	width_(width),
	height_(height),
	inited_(FALSE),
	window_({ 0 })
{
	g = this;
	window_ = { 0 };
	window_.cbClsExtra = NULL;
	window_.cbSize = sizeof(WNDCLASSEX);
	window_.cbWndExtra = NULL;
	window_.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	window_.hCursor = LoadCursor(0, IDC_ARROW);
	window_.hIcon = LoadIcon(0, IDI_APPLICATION);
	window_.hIconSm = LoadIcon(0, IDI_APPLICATION);
	window_.hInstance = GetModuleHandle(NULL);
	window_.lpfnWndProc = WindowProc;
	window_.lpszClassName = L"Microsoft Edge";
	window_.lpszMenuName = L"ZfDriver D3D";
	window_.style = CS_VREDRAW | CS_HREDRAW;


	RegisterClassEx(&window_);

	hwnd_ = CreateWindowEx
	(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		window_.lpszClassName,
		window_.lpszMenuName,
		WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_TABSTOP | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		1,
		1,
		width_,
		height_,
		HWND_DESKTOP,
		0,
		0,
		0
	);

	SetLayeredWindowAttributes(hwnd_, 0, RGB(0, 0, 0), LWA_COLORKEY);
	ShowWindow(hwnd_, SW_SHOW);

	// D3D
	pD3d_ = Direct3DCreate9(D3D_SDK_VERSION);
	d3dpp_.Windowed = TRUE;
	d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp_.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp_.EnableAutoDepthStencil = TRUE;
	d3dpp_.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	pD3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd_, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp_, &pD3dDevice_);
	D3DXCreateLine(pD3dDevice_, &pLine_);
	D3DXCreateFontW(pD3dDevice_, fontSize_, 0, FW_DONTCARE, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Vernada", &pFont_);

	drawThreadHandle_ = CreateThread(0, 0, D3D::FuncLoop, this, 0, 0);
	inited_ = TRUE;
	gW = GetSystemMetrics(SM_CXSCREEN);
	gH = GetScreenResolution();
}

D3D::~D3D()
{
	if (drawThreadHandle_)
	{
		CloseHandle(drawThreadHandle_);
	}
	if (pFont_ != NULL)
	{
		pFont_->Release();
		pFont_ = NULL;
	}

	if (pLine_ != NULL)
	{
		pLine_->Release();
		pLine_ = NULL;
	}

	if (pD3dDevice_ != NULL)
	{
		pD3dDevice_->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
		pD3dDevice_->Release();
		pD3dDevice_ = NULL;
	}

	if (pD3d_ != NULL)
	{
		pD3d_->Release();
		pD3d_ = NULL;
	}

	if (hwnd_ != NULL)
	{
		DestroyWindow(hwnd_);
		hwnd_ = NULL;
	}

	UnregisterClass(window_.lpszClassName, window_.hInstance);
}

INT D3D::GetFps()
{
	fps_.curFrames++;
	fps_.curTime = timeGetTime() * (FLOAT)0.001;
	if (fps_.curTime - fps_.lastTime > 1)
	{
		fps_.retFps = fps_.curFrames / (fps_.curTime - fps_.lastTime);
		fps_.lastTime = fps_.curTime;
		fps_.curFrames = 0;
	}
	return (INT)fps_.retFps;
}

VOID D3D::DrawFps(INT fontSize)
{
	std::wstringstream wss;
	wss << L"FPS: " << GetFps();
	DrawText(20, 20, wss.str().c_str(), D3DCOLOR_ARGB(255, 0, 255, 0), fontSize);
}

VOID D3D::DrawText(LONG x, LONG y, LPCWSTR str, D3DCOLOR color, INT fontSize)
{
	RECT rc = { x,y };
	ReCreateFont(fontSize);
	pFont_->DrawTextW(NULL, str, -1, &rc, DT_CALCRECT, color);
	pFont_->DrawTextW(NULL, str, -1, &rc, DT_LEFT, color);
}

VOID D3D::DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT lineWidth, D3DCOLOR color)
{
	D3DXVECTOR2 Vertex[2] = { {x1,y1},{x2,y2} };
	pLine_->SetWidth(lineWidth);
	pLine_->Draw(Vertex, 2, color);
}

VOID D3D::DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT lineWidth, D3DCOLOR color)
{
	D3DXVECTOR2 Vertex[5] = { {x,y},{x + width,y},{x + width,y + height},{x,y + height},{x,y} };
	pLine_->SetWidth(lineWidth);
	pLine_->Draw(Vertex, 5, color);
}

VOID D3D::DrawCircle(FLOAT x, FLOAT y, FLOAT r, D3DCOLOR color, LONG lineCount, FLOAT lineWidth)
{
	FLOAT step = (FLOAT)PI * 2 / lineCount;
	FLOAT size = (FLOAT)r;
	FLOAT tmp = 0;
	FLOAT x1, y1, x2, y2;
	for (; lineCount > 0; lineCount--)
	{
		x1 = size * cos(tmp) + x;
		y1 = size * sin(tmp) + y;
		x2 = size * cos(tmp + step) + x;
		y2 = size * sin(tmp + step) + y;
		tmp += step;
		DrawLine(x1, y1, x2, y2, lineWidth, color);
	}
}

VOID D3D::FillRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, D3DCOLOR color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	};

	Vertex V[8] = { 0 };
	V[0].Color = V[1].Color = V[2].Color = V[3].Color = color;
	V[0].z = V[1].z = V[2].z = V[3].z = 0.0f;
	V[0].ht = V[1].ht = V[2].ht = V[3].ht = 0.0f;
	V[0].x = V[1].x = (float)x;
	V[0].y = V[2].y = (float)(y + height);
	V[1].y = V[3].y = (float)y;
	V[2].x = V[3].x = (float)(x + width);
	pD3dDevice_->SetTexture(0, NULL);
	pD3dDevice_->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pD3dDevice_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
}

DWORD D3D::FuncLoop(LPVOID pD3DObject)
{
	D3D* obj = (D3D*)pD3DObject;
	timeBeginPeriod(1);
	while (obj->subFunc_)
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if (gW != GetSystemMetrics(SM_CXSCREEN) || gH != GetScreenResolution())
		{
			gW = GetSystemMetrics(SM_CXSCREEN);
			gH = GetScreenResolution();
			Reset();
			break;
		}

		obj->pD3dDevice_->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
		obj->pD3dDevice_->BeginScene();

		obj->subFunc_();

		obj->pD3dDevice_->EndScene();
		obj->pD3dDevice_->Present(0, 0, 0, 0);
	}
	timeEndPeriod(1);
	return 0;
}

VOID D3D::ReCreateFont(INT fontSize)
{
	if (fontSize != fontSize_)
	{
		if (pFont_)
		{
			pFont_->Release();
		}
		D3DXCreateFontW(pD3dDevice_, fontSize, 0, FW_DONTCARE, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Vernada", &pFont_);
	}
}
