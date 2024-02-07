#include <iostream>
#include "ZfDriver.h"

int main(void)
{
	ZfDriver::Install();

	// Test
	DWORD num = 1;
	std::cout << ZfDriver::Test(num) << std::endl;

	// Read
	ZfDriver::ReadBytes(5692, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	std::cout << num << std::endl;

	// Write
	num--;
	ZfDriver::WriteBytes(5692, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	ZfDriver::ReadBytes(5692, 0x000C6F38, sizeof(DWORD), (BYTE*)&num);
	std::cout << num << std::endl;

	ZfDriver::Uninstall();
	system("pause");
	return 0;
}