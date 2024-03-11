#include <atlstr.h>
#include <codecvt> 
#include <sstream>
#include "IMG.h"
#include "ZfDriver.h"

#pragma comment(lib,"opengl32.lib")


// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;
static ImVec4 clear_color = ImColor(0, 0, 0);
static ImGuiIO* gIo = 0;

// Helper

static std::string StringToUTF8(const std::string& gbkData)
{
	const char* GBK_LOCALE_NAME = "CHS";  //GBK在windows下的locale name(.936, CHS ), linux下的locale名可能是"zh_CN.GBK"

	std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>>
		conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
	std::wstring wString = conv.from_bytes(gbkData);    // string => wstring

	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	std::string utf8str = convert.to_bytes(wString);     // wstring => utf-8

	return utf8str;
}

static bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	const int pf = ::ChoosePixelFormat(hDc, &pfd);
	if (pf == 0)
		return false;
	if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
		return false;
	::ReleaseDC(hWnd, hDc);

	data->hDC = ::GetDC(hWnd);
	if (!g_hRC)
		g_hRC = wglCreateContext(data->hDC);
	return true;
}

static void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	wglMakeCurrent(nullptr, nullptr);
	::ReleaseDC(hWnd, data->hDC);
}

// Support function for multi-viewports
// Unlike most other backend combination, we need specific hooks to combine Win32+OpenGL.
// We could in theory decide to support Win32-specific code in OpenGL backend via e.g. an hypothetical ImGui_ImplOpenGL3_InitForRawWin32().
static void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
	assert(viewport->RendererUserData == NULL);

	WGL_WindowData* data = IM_NEW(WGL_WindowData);
	CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
	viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
{
	if (viewport->RendererUserData != NULL)
	{
		WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
		CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
		IM_DELETE(data);
		viewport->RendererUserData = NULL;
	}
}

static void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
	// Activate the platform window DC in the OpenGL rendering context
	if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
		wglMakeCurrent(data->hDC, g_hRC);
}

static void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
	if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
		::SwapBuffers(data->hDC);
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}


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
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceWGL(hwnd_, &g_MainWindow);
	wglDeleteContext(g_hRC);
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
	p->window_.lpfnWndProc = WindowProc;
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
	assert(hwnd_ != 0);

	// Initialize OpenGL
	if (!CreateDeviceWGL(p->hwnd_, &g_MainWindow))
	{
		CleanupDeviceWGL(p->hwnd_, &g_MainWindow);
		::DestroyWindow(p->hwnd_);
		::UnregisterClassW(p->window_.lpszClassName, p->window_.hInstance);
		return -1;
	}
	wglMakeCurrent(g_MainWindow.hDC, g_hRC);

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
	ImGui_ImplWin32_InitForOpenGL(p->hwnd_);
	ImGui_ImplOpenGL3_Init();

	// Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
		IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
		IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
		IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
		platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
		platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
		platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
		platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
	}

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

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Custom
		if (p->subFunc_)
			p->subFunc_();


		// Rendering
		ImGui::Render();
		glViewport(0, 0, g_Width, g_Height);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (gIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			// Restore the OpenGL rendering context to the main window DC, since platform windows might have changed it.
			wglMakeCurrent(g_MainWindow.hDC, g_hRC);
		}

		// Present
		::SwapBuffers(g_MainWindow.hDC);
	}
	return 0;
}
