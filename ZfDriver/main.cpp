#include "ZfDriver.h"

int main(void)
{
	Singleton<ZfDriver>::GetInstance().Install();
	Singleton<ZfDriver>::GetInstance().Uninstall();
	return 0;
}