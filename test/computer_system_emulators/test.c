#include <8052.h>

void main(void)
{
	P3 = 0x00;
	while (1) P1 = P3;
}

