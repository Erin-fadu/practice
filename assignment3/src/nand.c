
// nil_main() ������ nil_request_queue���� nil_request�� dequeue�Ͽ� nand�� ȣ���� ��
// nil_response�� �����, nil_response_queue�� enqueue��

#include <stdio.h>
#include <string.h>
#include "nand.h"
#include "nil.h"
#include "ftl.h"
#include "main.h"
#include "cqueue.h"

extern ssdconfig ssd_config;
nand_parameter nand_param;

unsigned char page_buffer_test_0xff[PAGE_BUFFER_SIZE];
unsigned char page_buffer_test_0[PAGE_BUFFER_SIZE];
unsigned char page_buffer_test_overwrite[PAGE_BUFFER_SIZE];

int flag_read = PAGE_READ_NO;
int flag_program = DMA_IN_NO;

int nand_init(void)
{
	printf("NAND: INITIAL\n");
	memset(&nand_param, 0, sizeof(nand_parameter));
	memset(&page_buffer_test_0xff, 0xff, PAGE_BUFFER_SIZE);
	memset(&page_buffer_test_0, 0, PAGE_BUFFER_SIZE);
	memset(&page_buffer_test_overwrite, 0, PAGE_BUFFER_SIZE);
	page_buffer_data_chunk=0;
	page_buffer_spare_chunk=0;
	chunk=0;
	return SUCCESS;
}

// program 1
int nand_dma_in(int channel, int lun, void *data_buffer, void *spare_buffer)
{
	if(flag_program == DMA_IN_NO)
	{
		if(memcmp(&nand_param.lun[lun].page_buffer, &page_buffer_test_0, PAGE_BUFFER_SIZE) == 0) // page buffer�� �ٸ� data�� ������ ����
		{
			printf("NAND: PROGRAM 1 [nand_dma_in()].\n");
			memcpy(&nand_param.lun[lun].page_buffer[page_buffer_data_chunk], data_buffer, LBA_SIZE);
			memcpy(&nand_param.lun[lun].page_buffer[page_buffer_spare_chunk], spare_buffer, SPARE_SIZE_PER_LBA);

			flag_program = DMA_IN_OK;
			return SUCCESS;
		}
		else
		{
			printf("NAND: PROGRAM 1 FAIL (Page Buffer is not EMPTY.)\n");
			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer ����
			flag_program = DMA_IN_NO;
			return FAIL;
		}
	}
	else
	{
		printf("NAND: PROGRAM 1 FAIL.\n");
		flag_program = DMA_IN_NO;
		return FAIL;
	}
}

// read 2
int nand_dma_out(int channel, int lun, void *data_buffer, void *spare_buffer)
{
	if(flag_read == PAGE_READ_OK)
	{
		if(memcmp(&nand_param.lun[lun].page_buffer, &page_buffer_test_0, PAGE_BUFFER_SIZE) == 0)
		{
			printf("NAND: READ 2 FAIL. (No data in Page Buffer.)\n");  // page buffer�� data�� ������ fail.
			flag_read = PAGE_READ_NO;
			return FAIL;
		}
		else
		{
			// page buffer ���� ������ host buffer�� ����
			printf("NAND: READ 2 [nand_dma_out()].\n");
			memcpy(data_buffer, &nand_param.lun[lun].page_buffer[page_buffer_data_chunk], LBA_SIZE);
			memcpy(spare_buffer, &nand_param.lun[lun].page_buffer[page_buffer_spare_chunk], SPARE_SIZE_PER_LBA);
			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer ����
			flag_read = PAGE_READ_NO;
			return SUCCESS;
		}
	}
	else
	{
		printf("NAND: READ 2 FAIL. (call nand_dma_out() before nand_page_read().)\n");
		flag_read = PAGE_READ_NO;
		return FAIL;
	}
}

// read 1
int nand_page_read(int channel, int lun, int block, int page)
{
	if(flag_read == PAGE_READ_NO)
	{
		if(page >= ssd_config.pages_per_block)
		{
			printf("NAND: READ 1 FAIL. (page size ERROR.)\n");
			return FAIL;
		}
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
			flag_read = PAGE_READ_NO;
			return -1;
		}

		int seek = page * (ssd_config.page_data_size + ssd_config.page_spare_size);	// page offset ����
		fseek(read_fp, seek, SEEK_SET);												// file pointer �̵�
		fread(&nand_param.lun[lun].page_buffer, PAGE_BUFFER_SIZE, 1, read_fp); // pointer�� ����Ű�� ������ page buffer�� ���

		if(memcmp(&nand_param.lun[lun].page_buffer, &page_buffer_test_0xff, PAGE_BUFFER_SIZE) == 0) //erased �˻�
		{
			printf("NAND: READ 1 FAIL. (Page is not programmed).\n");
			fclose(read_fp);
			memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer ����
			flag_read = PAGE_READ_NO;
			return ERASED;
		}
		printf("NAND: READ 1 [nand_page_read()] access path: %s\n", path);
		fclose(read_fp);
		flag_read = PAGE_READ_OK;
		return SUCCESS;
	}
	else
	{
		flag_read = PAGE_READ_NO;
		printf("NAND: READ 1 FAIL.\n");
		return FAIL;
	}
}

// program 2
int nand_page_program(int channel, int lun, int block, int page) // �ش� block.bin���� page�� �ش��ϴ� data��, page buffer�� data�� ����
{
	if(flag_program == DMA_IN_OK)
	{
		if(page >= ssd_config.pages_per_block)
		{
			printf("NAND: PROGRAM 2 FAIL. (page size ERROR.)\n");
			return FAIL;
		}
		if(nand_param.lun[lun].block[block] < page)
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
				flag_program = DMA_IN_NO;
				return -1;
			}
			int seek = page * (ssd_config.page_data_size + ssd_config.page_spare_size);	// �ش� page ã�ư��� ���� offset ����
			fseek(program_fp, seek, SEEK_SET);	// file pointer �̵�

			// overwrite �˻�
			fread(&page_buffer_test_overwrite, PAGE_BUFFER_SIZE, 1, program_fp);
			if(memcmp(&page_buffer_test_overwrite, &page_buffer_test_0xff, PAGE_BUFFER_SIZE) == 0) // overwrite�� �ƴϸ�
			{
				 // page buffer ������ block.bin�� ���
				fseek(program_fp, seek, SEEK_SET);	// file pointer �̵�
				fwrite(&nand_param.lun[lun].page_buffer, PAGE_BUFFER_SIZE, 1, program_fp);
				fclose(program_fp);
				nand_param.lun[lun].block[block] = page;
				memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer ����
				flag_program = DMA_IN_NO;
				return SUCCESS;
			}
			else // overwrite�̸�
			{
				printf("NAND: PROGRAM 2 FAIL. (Overwrite.)\n");
				// page buffer ����
				memset(&nand_param.lun[lun].page_buffer, 0, PAGE_BUFFER_SIZE); // page buffer ����
				fclose(program_fp);
				flag_program = DMA_IN_NO;
				return OVERWRITE;
			}
		}
		else
		{
			printf("NAND: PROGRAM 2 FAIL. (Program is not Sequential.)\n");
			flag_program = DMA_IN_NO;
			return FAIL;
		}
	}
	else
	{
		printf("NAND: PROGRAM 2 FAIL. (Call nand_page_program() before nand_dma_in().)\n");
		flag_program = DMA_IN_NO;
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

	printf("ERASE PATH: %s\n", path);

	FILE *fp = fopen(path, "wb");
	if(fp == NULL)
	{
		printf("block#%d.bin file open ERROR\n", block);
		return FAIL;
	}

	int block_size = ssd_config.pages_per_block * (ssd_config.page_data_size + ssd_config.page_spare_size);
	int init_num = 0xffffffff;


	for(i=0; i<(block_size/sizeof(init_num)); i++)
	{
		fwrite(&init_num, sizeof(init_num), 1, fp);
	}

	fclose(fp);

	return SUCCESS;
}

