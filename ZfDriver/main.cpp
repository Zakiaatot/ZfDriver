#include <iostream>
#include "ZfDriver.h"

int main(void)
{
	ZfDriver::Install();

	// Test
	DWORD num = 1;
	std::cout << ZfDriver::Test(num) << std::endl;

	// Read
	ZfDriver::ReadBytes(4868, 0x000BF140, sizeof(DWORD), (BYTE*)&num);
	std::cout << num << std::endl;

	// Write

	ZfDriver::Uninstall();
	system("pause");
	return 0;
}