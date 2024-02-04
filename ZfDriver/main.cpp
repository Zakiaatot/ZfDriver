#include "ZfDriver.h"

int main(void)
{
	Singleton<ZfDriver>::GetInstance().Install();
	Singleton<ZfDriver>::GetInstance().Uninstall();
	// Todo
	// Ö¤ÊéµõÏú
	return 0;
}