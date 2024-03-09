#include <iostream>
#include "ZfDriver.h"

#pragma comment(lib,"ZfDriver.lib")


VOID DrawLoop()
{
	ZfDriver::D3DDrawFps(24);
	ZfDriver::D3DDrawRect(100, 100, 100, 100, 2, D3DCOLOR_RGB(255, 0, 0));
	ZfDriver::D3DDrawRectFill(500, 500, 200, 200, D3DCOLOR_RGB(0, 0, 255));
	ZfDriver::D3DDrawText(0, 0, L"∑…∏ÁªÊ÷∆", D3DCOLOR_RGB(255, 255, 0), 24);
	ZfDriver::D3DDrawCircle(300, 100, 50, D3DCOLOR_RGB(0, 255, 0), 8, 2);
	ZfDriver::D3DDrawLine(300, 100, 500, 500, 1, D3DCOLOR_RGB(0, 255, 255));
}

int main(void)
{
	ZfDriver::Install();

	// Test
	//DWORD num = 1;
	//std::cout << ZfDriver::Test(num) << std::endl;

	//// Read
	//ZfDriver::ReadBytes(5692, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	//std::cout << num << std::endl;

	//// Write
	//num--;
	//DWORD pid = ZfDriver::GetProcessId(L"explorer.exe");
	//ZfDriver::WriteBytes(pid, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	//ZfDriver::ReadBytes(pid, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	//std::cout << num << std::endl;

	// Force Delete File
	//ZfDriver::ForceDeleteFile(L"C:\\Users\\38463\\Desktop\\Project\\ZfDriver\\ZfDriver\\x64\\Release\\ZfDriver.exe");

	// Get Module Base
	//int pid = 0;
	//while (true)
	//{
	//	std::cout << "Pid: ";
	//	std::cin >> pid;
	//	if (pid == 0)
	//		break;
	//	std::cout << std::hex << ZfDriver::GetModuleBase(pid, L"ntdll.dll") << std::endl;
	//}

	// Keyboard and Mouse
	//ZfDriver::MouseRightButtonDown();
	//ZfDriver::MouseMoveAbsolute(100, 600);
	//ZfDriver::KeyDown(0x14); // 0x14: Caps lock

	// Process Hide
	//int pid = 0;
	//while (true)
	//{
	//	std::cout << "Pid: ";
	//	std::cin >> pid;
	//	if (pid == 0)
	//		break;
	//	std::cout << ZfDriver::ProcessHide(pid) << std::endl;
	//}

	// Window Hide
	//HWND hwnd = FindWindowW(L"RegEdit_RegEdit", 0);
	//if (hwnd)
	//{
	//	std::cout << "Hwnd: 0x" << std::hex << hwnd << std::endl;
	//	std::cout << ZfDriver::WindowHide(hwnd) << std::endl;
	//}

	// Get Process Id
	//std::cout << ZfDriver::GetProcessId(L"explorer.exe") << std::endl;

	// Inject Dll
	//DWORD pid = ZfDriver::GetProcessId(L"explorer.exe");
	//if (pid > 0)
	//{
	//	std::cout << ZfDriver::InjectDll(pid, L"C:\\Users\\38463\\Desktop\\test.dll") << std::endl;
	//}

	// Draw
	BOOL init = ZfDriver::D3DDrawInit(DrawLoop);
	if (init)
	{
		std::cout << "Draw inited." << std::endl;
		while (true)
		{
			Sleep(1);
		};
	}

	ZfDriver::Uninstall();
	system("pause");
	return 0;
}

