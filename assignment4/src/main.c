// 과제 3: NAND emulation - erin
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "main.h"
#include "cqueue.h"
#include "ftl.h"
#include "nil.h"
#include "nand.h"

ssdconfig ssd_config;
host_buffer_manager host_buf_manager;

extern cqueue ftl_response_queue, ftl_request_queue;
extern ftl_request_packet  ftl_request;
extern ftl_response_packet ftl_response;

extern unsigned int *page_table;
extern struct list_head valid_page_count[MAX_VALID_PAGE_COUNT];

short host_alloc_buffer(void) // 현재 사용하지 않는 host_buffer_id 할당하여 return
{
	short id = 0;
	if(host_buf_manager.num_used < host_buf_manager.buffer_count)
	{
		for(id=0; id<host_buf_manager.buffer_count; id++)
		{
			if(host_buf_manager.buffers[id].used == 0)
			{
				host_buf_manager.buffers[id].used = 1;
				host_buf_manager.num_used++;
				printf("HOST: id %d is allocated.\n", id);
				return id;
			}
			else
			{
				continue;
			}
		}
	}
	else
		return 0xffff;
}

int host_release_buffer(short id)
{
	if(host_buf_manager.buffers[id].used == 1)
	{
		host_buf_manager.buffers[id].used = 0;
		host_buf_manager.num_used--;
		memset(&host_buf_manager.buffers[id].data, 0, LBA_SIZE);
		memset(&host_buf_manager.buffers[id].spare, 0, SPARE_SIZE_PER_LBA);
		printf("HOST: ID %d is released.\n", id);
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}

void* host_get_data_address(short id)
{
	if(host_buf_manager.buffers[id].used == 1 && host_buf_manager.buffers[id].data != NULL)
		return host_buf_manager.buffers[id].data;
	else
		return FAIL;
}

void* host_get_spare_address(short id)
{
	if(host_buf_manager.buffers[id].used == 1 && host_buf_manager.buffers[id].spare != NULL)
		return host_buf_manager.buffers[id].spare;
	else
		return FAIL;
}

int str_to_int(FILE *fp, char *str)
{
	char *val;
	fscanf(fp, "%s", str);
	val = strtok(str, "'");
	return atoi(val);
}

int ssd_config_init(void)
{
	char strline[20];
	char chstr[20], lunstr[20], blockstr[20], num[10];
	int i=0, j=0, k=0, l=0;
	FILE *bp;

	FILE *fp = fopen("ssd_config.txt", "r");
	if(fp == NULL)
	{
		printf("Fail to open file.\n");
		return FAIL;
	}

	while(!feof(fp)) // 한 줄씩 받아옴
	{
		fscanf(fp, "%s", strline);

		if(strcmp(strline, "'channel_count'") == 0)
		{
			ssd_config.channel_count = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'luns_per_channel'") == 0)
		{
			ssd_config.luns_per_channel = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'blocks_per_lun'") == 0)
		{
			ssd_config.blocks_per_lun = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'pages_per_block'")==0)
		{
			ssd_config.pages_per_block = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'page_data_size'")==0)
		{
			ssd_config.page_data_size = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'page_spare_size'")==0)
		{
			ssd_config.page_spare_size = str_to_int(fp, strline);
		}
		else if(strcmp(strline, "'lba_count'")==0)
		{
			ssd_config.lba_count = str_to_int(fp, strline);
		}
	}
	fclose(fp);

	// block 크기 지정 : page 개수 * (data 영역 크기 + spare 영역 크기)
	int block_size = ssd_config.pages_per_block * (ssd_config.page_data_size + ssd_config.page_spare_size);
	int n = 0xffffffff;

	// make directory
	mkdir("./data", 0755);
	for(i=0; i<ssd_config.channel_count; i++)
	{
		strcpy(chstr, "./data/ch#"); 	// copy channel directory to channel string
		sprintf(num, "%d", i);			// channel_count string to integer
		strcat(chstr, num);				// channel string + channel count ./data/ch#
		mkdir(chstr, 0755);				// make channel directory
		strcat(chstr, "/lun#");

		for(j=0; j<ssd_config.luns_per_channel; j++)
		{
			strcpy(lunstr, chstr);		// lun string : ./data/ch#x/lun#
			sprintf(num, "%d", j);
			strcat(lunstr, num);		// ./data/ch#x/lun#y
			mkdir(lunstr, 0755);
			strcat(lunstr, "/block#");

			for(k=0; k<ssd_config.blocks_per_lun; k++)
			{
				strcpy(blockstr, lunstr);	// ./data/ch#x/lun#y/block#
				sprintf(num, "%d", k);
				strcat(blockstr, num);		// ./data/ch#x/lun#y/block#z
				strcat(blockstr, ".bin");	// ./data/ch#x/lun#y/block#z.bin
				bp = fopen(blockstr, "wb");	// make block file
				if(bp == NULL)
				{
					printf("block#%d.bin file open ERROR\n", k);
					return -1;
				}
				for(l=0; l<(block_size/sizeof(n)); l++)
				{
					fwrite(&n, sizeof(n), 1, bp);
				}
				fclose(bp);
			}
		}
	}
	return SUCCESS;
}

int init(void)
{
	printf("MAIN: INITIAL.\n");

	if(ssd_config_init() != SUCCESS)
		return FAIL;

	if(nil_init() != SUCCESS || nand_init() != SUCCESS)
		return FAIL;

	if(ftl_init() != SUCCESS)
		return FAIL;

	// host buffer initialization
	host_buf_manager.buffer_count = MAX_HOST_BUFFERS;
	host_buf_manager.num_used = 0;
	memset(&host_buf_manager.buffers, 0, MAX_HOST_BUFFERS * (LBA_SIZE + SPARE_SIZE_PER_LBA+ sizeof(int)));
	return SUCCESS;

}

void host_buffer_print(host_buffer_manager *buf, short id)
{
	printf("| [ HOST BUFFER STRUCT (id: %d) ]\n", id);
	int i =0;
	printf("|   Data: ");
	for(i=0; i<sizeof(int); i++)
	{
		printf("%x ", buf->buffers[id].data[i]);
	}
	printf("\n|   Spare: ");
	for(i=0; i<sizeof(int); i++)
	{
		printf("%x ", buf->buffers[id].spare[i]);
	}
	printf("\n");
}

void host_write_request(int lba)
{
	short id=0, id_fail = 0xffff;
	int buf_data, buf_spare;

	printf("------------------------------------------------------\n");
	id = host_alloc_buffer();
	if(cqueue_test(&ftl_request_queue)!=FULL && id!=id_fail)
	{
		buf_data = rand()%100000000;
		buf_spare = rand()%100000000;
		printf("HOST: Make WRITE REQUEST packet, and Enqueue. (ID: %d)\n", id);
		memcpy(&host_buf_manager.buffers[id].data, &buf_data, sizeof(buf_data));
		memcpy(&host_buf_manager.buffers[id].spare, &buf_spare, sizeof(buf_spare));
		host_buffer_print(&host_buf_manager, id);

		// ftl_request write packet 생성
		ftl_request.opcode = OP_WRITE;
		ftl_request.id = id;
		ftl_request.lba = lba;

		if(cqueue_enqueue(&ftl_request_queue, &ftl_request) != SUCCESS)
			printf("HOST: Request Enqueue ERROR.\n");
	}
	else
	{
		printf("HOST: FAIL Enqueue write request.\n");
	}
}

void host_read_request(int lba)
{
	short id=0, id_fail = 0xffff;

	printf("------------------------------------------------------\n");
	id = host_alloc_buffer();
	if(cqueue_test(&ftl_request_queue)!=FULL &&  id!=id_fail)
	{
		printf("HOST: Make READ REQUEST packet, and Enqueue. (ID: %d)\n", id); // ftl_request packet 생성
		ftl_request.opcode = OP_READ;
		ftl_request.id = id;
		ftl_request.lba = lba;

		if(cqueue_enqueue(&ftl_request_queue, &ftl_request) != SUCCESS)
			printf("HOST: Request Enqueue ERROR.\n");
	}
	else
	{
		printf("HOST: FAIL Enqueue read request.\n");
	}
}

void host_response(void)
{
	if(cqueue_test(&ftl_response_queue) != EMPTY) // ftl_response_queue가 not empty이면,
	{
		printf("------------------------------------------------------\n");
		// ftl_response를 dequeue
		cqueue_dequeue(&ftl_response_queue, &ftl_response);
		if(ftl_response.status == 0)
		{
			printf("HOST: [RESULT ID: %d]\n", ftl_response.id);
			host_buffer_print(&host_buf_manager, ftl_response.id);
		}
		else
		{
			printf("HOST: [RESULT ID: %d] READ/WRITE FAIL.\n", ftl_response.id);
		}

		if(host_release_buffer(ftl_response.id) == FAIL) //   id 해제(host_release_buffer), error 출력
			printf("HOST: host_release_buffer ERROR (ID: %d).\n", ftl_response.id);
	}
}



int main()
{
	int ran=0;
	srand(time(NULL));

	if(init() == FAIL)
	{
		printf("INITIAL ERROR.\n");
		return -1;
	}

	int a=0;
	while(a<15000)
	{
		a++;
		host_response();
		ran = rand()%4;
		switch(ran)
		{
		case 0: //20%의 확률로, ftl_request_queue가 not full이고 id 할당 받을 수 있으면(host_alloc_buffer 호출해서 return 값이 0xFFFF가 아니면)
			host_read_request(rand()%ssd_config.lba_count);
			break;
		case 1:	// 20%의 확률로, ftl_request_queue가 not full이면 id 할당 받을 수 있으면
			host_write_request(rand()%ssd_config.lba_count);
			break;
		case 2:
			ftl_main();
			break;
		case 3:
			nil_main();
			break;
		default:
			break;
		}
	}

	return 0;
}
