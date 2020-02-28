#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ftl.h"
#include "nand.h"

int main()
{
	srand(time(NULL));
	int ran=0;
	unsigned int address =rand()%1000000000;

	ftl_init();
	nand_init();

	while(1)
	{
		ran = rand()%10;
		switch(ran)
		{
		case 0:				// 20%의 확률로, 사용자의 입력을 받아 ftl_read() 호출
		case 1:
			address = rand()%1000000000;
			ftl_read(address);
			break;
		case 2:				// 20%의 확률로, 사용자의 입력을 받아 ftl_write() 호출
		case 3:
			address = rand()%1000000000;
			ftl_write(address);
			break;
		case 4:				// 30%의 확률로, ftl_main() 호출
		case 5:
		case 6:
			ftl_main();
			break;
		case 7:				// 30%의 확률로, nand_main() 호출
		case 8:
		case 9:
			nand_main();
			break;
		default:
			break;
		}
	}
	return 0;
}
