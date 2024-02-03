#include "ZfDriver.h"

int main(void)
{
	Singleton<ZfDriver>::GetInstance().Install();
	return 0;
}