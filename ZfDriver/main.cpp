#include <iostream>
#include "ZfDriver.h"

int main(void)
{
	ZfDriver::Install();
	DWORD num = 1;
	std::cout << ZfDriver::Test(num) << std::endl;
	ZfDriver::Uninstall();
	return 0;
}