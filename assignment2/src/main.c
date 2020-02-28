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
		case 0:				// 20%�� Ȯ����, ������� �Է��� �޾� ftl_read() ȣ��
		case 1:
			address = rand()%1000000000;
			ftl_read(address);
			break;
		case 2:				// 20%�� Ȯ����, ������� �Է��� �޾� ftl_write() ȣ��
		case 3:
			address = rand()%1000000000;
			ftl_write(address);
			break;
		case 4:				// 30%�� Ȯ����, ftl_main() ȣ��
		case 5:
		case 6:
			ftl_main();
			break;
		case 7:				// 30%�� Ȯ����, nand_main() ȣ��
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
