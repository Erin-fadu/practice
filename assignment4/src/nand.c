
// nil_main() 에서는 nil_request_queue에서 nil_request를 dequeue하여 nand를 호출한 뒤
// nil_response를 만들어, nil_response_queue에 enqueue함

#include <stdio.h>
#include <string.h>
#include "nand.h"
#include "nil.h"
#include "ftl.h"
#include "main.h"
#include "cqueue.h"

extern ssdconfig ssd_config;
nand_parameter nand_param;

int dma_in_count;
int dma_out_count;
int flag_program, flag_read;

int nand_init(void)
{
	printf("NAND: INITIAL\n");
	memset(&nand_param, 0, sizeof(nand_parameter));
	dma_in_count=0;
	dma_out_count=0;
	flag_program = READY_DMA_IN;
	flag_read = READY_PAGE_READ;

	return SUCCESS;
}

// program 1
int nand_dma_in(int channel, int lun, int chunk, void *data_buffer, void *spare_buffer)
{
	int data_chunk, spare_chunk;
	if(flag_program == READY_DMA_IN)
	{
		printf("NAND: PROGRAM 1 [nand_dma_in()] chunk %d.\n", chunk);
		data_chunk = chunk*LBA_SIZE;
		spare_chunk = ssd_config.page_data_size + (chunk * SPARE_SIZE_PER_LBA);
		memcpy(&nand_param.lun[lun].page_buffer[data_chunk], data_buffer, LBA_SIZE);
		memcpy(&nand_param.lun[lun].page_buffer[spare_chunk], spare_buffer, SPARE_SIZE_PER_LBA);
		if(dma_in_count == 3)
		{
			dma_in_count = 0;
			flag_program = READY_PAGE_PROGRAM;
			return SUCCESS;
		}
		dma_in_count++;
		return SUCCESS;
	}
	else
	{
		printf("NAND: PROGRAM 1 FAIL.\n");
		flag_program = READY_DMA_IN;
		return FAIL;
	}
}

// read 2
int nand_dma_out(int channel, int lun, int chunk, int used_count, void *data_buffer, void *spare_buffer)
{
	int data_chunk, spare_chunk;
	if(flag_read == READY_DMA_OUT)
	{
		// page buffer 안의 내용을 host buffer로 전송
		printf("NAND: READ 2 [nand_dma_out()] chunk %d.\n", chunk);
		data_chunk = chunk*LBA_SIZE;
		spare_chunk = ssd_config.page_data_size + (chunk * SPARE_SIZE_PER_LBA);
		memcpy(data_buffer, &nand_param.lun[lun].page_buffer[data_chunk], LBA_SIZE);
		memcpy(spare_buffer, &nand_param.lun[lun].page_buffer[spare_chunk], SPARE_SIZE_PER_LBA);
		dma_out_count++;
		if(dma_out_count == used_count)
		{
			dma_out_count = 0;
			flag_read = READY_PAGE_READ;
			return SUCCESS;
		}
		flag_read = READY_DMA_OUT;
		return SUCCESS;
	}
	else
	{
		printf("NAND: READ 2 FAIL. (call nand_dma_out() before nand_page_read().)\n");
		flag_read = READY_PAGE_READ;
		return FAIL;
	}
}

// read 1
int nand_page_read(int channel, int lun, int block, int page)
{
	int seek;
	if(flag_read == READY_PAGE_READ)
	{
		char path[50] = "./data/ch#";
		char ch_num[10], lun_num[10], block_num[10];
		sprintf(ch_num, "%d", channel);
		sprintf(lun_num, "%d", lun);
		sprintf(block_num, "%d", block);
		strcat(path, ch_num);
		strcat(path, "/lun#");
		strcat(path, lun_num);
		strcat(path, "/block#");
		strcat(path, block_num);
		strcat(path, ".bin");

		FILE *read_fp = fopen(path, "rb");
		if(read_fp == NULL)
		{
			printf("NAND: READ 1 FAIL. (block#%d.bin file open ERROR.)\n", block);
			flag_read = READY_PAGE_READ;
			return -1;
		}

		if(page <= nand_param.lun[lun].block_page_pointer[block]) // erased 검사
		{
			seek = page * (ssd_config.page_data_size + ssd_config.page_spare_size);	// page offset 구함
			fseek(read_fp, seek, SEEK_SET);												// file pointer 이동
			fread(&nand_param.lun[lun].page_buffer, PAGE_BUFFER_SIZE, 1, read_fp); // pointer가 가르키는 내용을 page buffer에 기록

			printf("NAND: READ 1 [nand_page_read()] access path: %s\n", path);
			fclose(read_fp);
			flag_read = READY_DMA_OUT;
			return SUCCESS;
		}
		else
		{
			printf("NAND: READ 1 FAIL. (Page is not programmed).\n");
			fclose(read_fp);
			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer 비우기
			flag_read = READY_PAGE_READ;
			return ERASED;
		}
	}
	else
	{
		flag_read = READY_PAGE_READ;
		printf("NAND: READ 1 FAIL.\n");
		return FAIL;
	}
}

// program 2
int nand_page_program(int channel, int lun, int block, int page) // 해당 block.bin에서 page에 해당하는 data을, page buffer의 data로 변경
{
	int seek;
	if(flag_program == READY_PAGE_PROGRAM)
	{
		char path[50] = "./data/ch#";
		char ch_num[10], lun_num[10], block_num[10];
		sprintf(ch_num, "%d", channel);
		sprintf(lun_num, "%d", lun);
		sprintf(block_num, "%d", block);
		strcat(path, ch_num);
		strcat(path, "/lun#");
		strcat(path, lun_num);
		strcat(path, "/block#");
		strcat(path, block_num);
		strcat(path, ".bin");
		printf("NAND: PROGRAM 2 [nand_page_program()] access path: %s\n", path);

		FILE *program_fp = fopen(path, "r+b");	// block.bin open
		if(program_fp == NULL)
		{
			printf("NAND: PROGRAM 2 FAIL. (block#%d.bin file open ERROR.)\n", block);
			flag_program = READY_DMA_IN;
			return -1;
		}

		if(page == 0 || page > nand_param.lun[lun].block_page_pointer[block]) // overwrite 검사
		{
			seek = page * (ssd_config.page_data_size + ssd_config.page_spare_size);	// 해당 page 찾아가기 위해 offset 구함
			fseek(program_fp, seek, SEEK_SET);	// file pointer 이동
			fwrite(&nand_param.lun[lun].page_buffer, PAGE_BUFFER_SIZE, 1, program_fp);
			fclose(program_fp);

			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer 비우기
			nand_param.lun[lun].block_page_pointer[block] = page;
			flag_program = READY_DMA_IN;
			return SUCCESS;
		}
		else
		{
			printf("NAND: PROGRAM 2 FAIL. (Overwrite.)\n");
			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer 비우기
			fclose(program_fp);
			flag_program = READY_DMA_IN;
			return OVERWRITE;
		}
	}
	else
	{
		printf("NAND: PROGRAM 2 FAIL. (Call nand_page_program() before nand_dma_in().)\n");
		flag_program = READY_DMA_IN;
		return FAIL;
	}

}



int nand_block_erase(int channel, int lun, int block)
{
	int i=0;
	char path[50] = "./data/ch#";
	char ch_num[10], lun_num[10], block_num[10];

	sprintf(ch_num, "%d", channel);
	sprintf(lun_num, "%d", lun);
	sprintf(block_num, "%d", block);
	strcat(path, ch_num);
	strcat(path, "/lun#");
	strcat(path, lun_num);
	strcat(path, "/block#");
	strcat(path, block_num);
	strcat(path, ".bin");

	printf("NAND: ERASE block (PATH: %s)\n", path);

	FILE *fp = fopen(path, "wb");
	if(fp == NULL)
	{
		printf("NAND: block#%d.bin file open ERROR\n", block);
		return FAIL;
	}

	int block_size = ssd_config.pages_per_block * (ssd_config.page_data_size + ssd_config.page_spare_size);
	int init_num = 0xffffffff;


	for(i=0; i<(block_size/sizeof(init_num)); i++)
	{
		fwrite(&init_num, sizeof(init_num), 1, fp);
	}

	fclose(fp);
	nand_param.lun[lun].block_page_pointer[block] = 0; // 현재 block에서 사용된 page 개수를 0으로 저장.

	return SUCCESS;
}

