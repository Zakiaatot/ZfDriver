#include "ZfDriver.h"

int main(void)
{
	Singleton<ZfDriver>::GetInstance().Install();
	Singleton<ZfDriver>::GetInstance().Uninstall();
	// Todo
	// ֤�����
	return 0;
}