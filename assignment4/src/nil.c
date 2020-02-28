// 해당 ch#x/lun#y/block#z.bin 에 접근하여 read, program, erase를 수행

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nil.h"
#include "cqueue.h"
#include "nand.h"
#include "main.h"
#include "ftl.h"

cqueue nil_request_queue, nil_response_queue;
nil_request_packet nil_request;
nil_response_packet nil_response;

extern _ftl_merge_manager ftl_merge_manager;

int nil_init(void)
{
	printf("NIL: INITIAL\n");
	memset(&nil_request, 0, sizeof(nil_request_packet));
	memset(&nil_response, 0, sizeof(nil_response_packet));

	if(cqueue_init(&nil_request_queue, sizeof(nil_request_packet)) == FAIL)
		return FAIL;
	if(cqueue_init(&nil_response_queue, sizeof(nil_response_packet)) == FAIL)
		return FAIL;

	return SUCCESS;
}

void nil_main(void)
{
	printf("------------------------------------------------------\n");
	int channel, lun, block, page, chunk;
	int merge_index, merge_id, used_count;
	int result1 = SUCCESS, result2 = SUCCESS;

	if(cqueue_test(&nil_response_queue) != FULL && cqueue_test(&nil_request_queue) != EMPTY)
	{
		assert(cqueue_dequeue(&nil_request_queue, &nil_request) == SUCCESS);
		channel = (nil_request.page_address & 0x3c000000) >> 26;
		lun = (nil_request.page_address & 0x03c00000) >> 22;
		block = (nil_request.page_address & 0x003ffe00) >> 9;
		page = nil_request.page_address & 0x000001ff;
		merge_index = nil_request.id;

		if(nil_request.OPC == PROGRAM)
		{
			printf("NIL: PROGRAM REQUEST. (entry: %d)\n", nil_request.id);
			for(int i=0; i<MAX_MUS_PER_PAGE; i++) // lba를 page buffer에 채워 넣음.
			{
				chunk = (ftl_merge_manager.entries[merge_index].entries[i].mpa & 0x00000003);
				merge_id = ftl_merge_manager.entries[merge_index].entries[i].id;
				if(nand_dma_in(channel, lun, chunk, host_get_data_address(merge_id), host_get_spare_address(merge_id)) != SUCCESS)
				{
					result1 = FAIL;
					break;
				}
			}
			// nand에 program
			if(result1 == SUCCESS)
			{
				result2 = nand_page_program(channel, lun, block, page);
				// 완료 메시지 출력
				printf("NIL: NAND is Updated. \n");
				printf("| channel: %d, lun: %d, block: %d, page: %d\n", channel, lun, block, page);
				printf("NIL: Make PROGRAM RESPONSE packet, and Enqueue.(ID: %d %d %d %d)\n", ftl_merge_manager.entries[merge_index].entries[0].id,
						ftl_merge_manager.entries[merge_index].entries[1].id, ftl_merge_manager.entries[merge_index].entries[2].id,
						ftl_merge_manager.entries[merge_index].entries[3].id);
			}
		}
		else if(nil_request.OPC == READ)
		{
			printf("NIL: READ REQUEST. (entry: %d)\n", nil_request.id);
			if(nand_page_read(channel, lun, block, page) == SUCCESS)
			{
				used_count = ftl_merge_manager.entries[merge_index].used;
				for(int i=0; i < ftl_merge_manager.entries[merge_index].used; i++)
				{
					chunk = (ftl_merge_manager.entries[merge_index].entries[i].mpa & 0x00000003);
					merge_id = ftl_merge_manager.entries[merge_index].entries[i].id;
					if(nand_dma_out(channel, lun, chunk, used_count, host_get_data_address(merge_id), host_get_spare_address(merge_id)) != SUCCESS)
					{
						result2 = FAIL;
						break;
					}
				}
			}
			// 완료 메시지 출력
			printf("| channel: %d, lun: %d, block: %d, page: %d\n", channel, lun, block, page);
			printf("NIL: Make PROGRAM RESPONSE packet, and Enqueue. (ID:");
			for(int i=0; i < ftl_merge_manager.entries[merge_index].used; i++)
			{
				printf(" %d", ftl_merge_manager.entries[merge_index].entries[i].id);
			}
			printf(")\n");
		}
		else if(nil_request.OPC == ERASE)
		{
			printf("NIL: ERASE.\n");
			if(nand_block_erase(channel, lun, block)!=SUCCESS)
				result2 = FAIL;
		}
		else
		{
			printf("NIL: REQUEST OPC ERROR.\n");
			assert(0);
		}
		// response packet 만듦
		nil_response.id = nil_request.id;
		if(result2 == SUCCESS)
			nil_response.status = 0;
		else
			nil_response.status = 1;
		assert(cqueue_enqueue(&nil_response_queue, &nil_response) == SUCCESS);
	}
	else
	{
		printf("NIL: REQUEST/RESPONSE FAIL. wait for next NIL_MAIN.\n");
	}
}









