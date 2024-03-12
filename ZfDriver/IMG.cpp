#include <atlstr.h>
#include <codecvt> 
#include <sstream>
#include "IMG.h"
#include "ZfDriver.h"

#pragma comment(lib,"d3d11.lib")

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static ImGuiIO* gIo = 0;
static ImVec4 clear_color = ImColor(0, 0, 0);

// Forward declarations of helper functions
static bool CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static std::string StringToUTF8(const std::string& gbkData);

IMG::IMG(LONG width, LONG height, SUB_FUNC subFunc, INT fontSize)
	:
	width_(width),
	height_(height),
	subFunc_(subFunc),
	fontSize_(fontSize),
	hwnd_(0),
	window_({ 0 }),
	inited_(FALSE),
	drawThreadHandle_(0)
{
	// Start Draw Thread
	drawThreadHandle_ = CreateThread(0, 0, FuncLoop, this, 0, 0);
	inited_ = TRUE;
}

IMG::~IMG()
{
	CloseHandle(drawThreadHandle_);
	subFunc_ = 0;
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd_);
	::UnregisterClassW(window_.lpszClassName, window_.hInstance);
}

INT IMG::GetFps()
{
	return (INT)gIo->Framerate;
}

VOID IMG::DrawFps()
{
	std::wstringstream fps;
	fps << L"Fps: " << GetFps();
	DrawText(20, 20, fps.str().c_str(), ImColor(0, 255, 0));
}

VOID IMG::DrawText(FLOAT x, FLOAT y, LPCWSTR str, ImColor color)
{
	CW2A cw2a(str);
	std::string string(cw2a);
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), color, StringToUTF8(string).c_str());
}

VOID IMG::DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT lineWidth, ImColor color)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, lineWidth);
}

VOID IMG::DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT lineWidth, ImColor color)
{
	ImGui::GetForegroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), color, 0, 0, lineWidth);
}

VOID IMG::DrawCircle(FLOAT x, FLOAT y, FLOAT r, ImColor color, LONG lineCount, FLOAT lineWidth)
{
	ImGui::GetForegroundDrawList()->AddCircle(ImVec2(x, y), r, color, lineCount, lineWidth);
}

VOID IMG::FillRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, ImColor color)
{
	ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), color);
}

DWORD IMG::FuncLoop(LPVOID pIMGObject)
{
	IMG* p = (IMG*)pIMGObject;

	p->window_.cbClsExtra = NULL;
	p->window_.cbSize = sizeof(WNDCLASSEX);
	p->window_.cbWndExtra = NULL;
	p->window_.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	p->window_.hCursor = LoadCursor(0, IDC_ARROW);
	p->window_.hIcon = LoadIcon(0, IDI_APPLICATION);
	p->window_.hIconSm = LoadIcon(0, IDI_APPLICATION);
	p->window_.hInstance = GetModuleHandle(NULL);
	p->window_.lpfnWndProc = WndProc;
	p->window_.lpszClassName = L"Microsoft Edge";
	p->window_.lpszMenuName = L"ZfDriver IMG";
	p->window_.style = CS_VREDRAW | CS_HREDRAW;


	RegisterClassEx(&p->window_);

	p->hwnd_ = CreateWindowEx
	(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		p->window_.lpszClassName,
		p->window_.lpszMenuName,
		WS_POPUP | WS_OVERLAPPED | WS_SYSMENU | WS_TABSTOP | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		0,
		0,
		p->width_,
		p->height_,
		HWND_DESKTOP,
		0,
		0,
		0
	);
	assert(p->hwnd_ != 0);

	// Initialize Direct3D
	if (!CreateDeviceD3D(p->hwnd_))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(p->window_.lpszClassName, p->window_.hInstance);
		return -1;
	}

	// Show the window
	::ShowWindow(p->hwnd_, SW_SHOWDEFAULT);
	::UpdateWindow(p->hwnd_);
	SetLayeredWindowAttributes(p->hwnd_, ImColor(0, 0, 0), NULL, LWA_COLORKEY);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io; gIo = &ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(p->hwnd_);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// Load Fonts
	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", p->fontSize_, nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	IM_ASSERT(font != nullptr);

	// Hide Window
	ZfDriver::WindowHide(p->hwnd_);

	// Main loop
	while (p->subFunc_)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Custom
		if (p->subFunc_)
			p->subFunc_();


		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		//g_pSwapChain->Present(1, 0); // Present with vsync
		g_pSwapChain->Present(0, 0); // Present without vsync
	}
	return 0;
}


// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static std::string StringToUTF8(const std::string& gbkData)
{
	const char* GBK_LOCALE_NAME = "CHS";

	std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>>
		conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
	std::wstring wString = conv.from_bytes(gbkData);    // string => wstring

	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	std::string utf8str = convert.to_bytes(wString);     // wstring => utf-8

	return utf8str;
}