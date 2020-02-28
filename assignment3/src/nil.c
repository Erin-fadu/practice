// 해당 ch#x/lun#y/block#z.bin 에 접근하여 read, program, erase를 수행

#include <stdio.h>
#include <string.h>
#include "nil.h"
#include "cqueue.h"
#include "nand.h"
#include "main.h"

cqueue nil_request_queue, nil_response_queue;
nil_request_packet nil_request;
nil_response_packet nil_response;

extern ssdconfig ssd_config;
extern int page_buffer_data_chunk, page_buffer_spare_chunk, chunk;

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
	int channel, lun, block, page;
	int result = FAIL;
	int hex_viewer_data, hex_viewer_spare;
	if(cqueue_test(&nil_response_queue) != FULL && cqueue_test(&nil_request_queue) != EMPTY)
	{
		cqueue_dequeue(&nil_request_queue, &nil_request);

		channel = (nil_request.page_address & 0x3c000000) >> 26;
		lun = (nil_request.page_address & 0x03c00000) >> 22;
		block = (nil_request.page_address & 0x003ffe00) >> 9;
		page = nil_request.page_address & 0x000001ff;

		printf("| channel: %d, lun: %d, block: %d, page: %d, 4k_chunk_validity_vector: %d\n", channel, lun, block, page, nil_request.fourk_chunk_validity_vector);
		if(nil_request.fourk_chunk_validity_vector == 1)
			chunk=0;
		else if(nil_request.fourk_chunk_validity_vector == 2)
			chunk=1;
		else if(nil_request.fourk_chunk_validity_vector == 4)
			chunk=2;
		else if(nil_request.fourk_chunk_validity_vector == 8)
			chunk=3;
		page_buffer_data_chunk = chunk * LBA_SIZE;
		page_buffer_spare_chunk = ssd_config.page_data_size + (chunk * SPARE_SIZE_PER_LBA);
		hex_viewer_data = page * (ssd_config.page_data_size + ssd_config.page_spare_size) + page_buffer_data_chunk;
		hex_viewer_spare = page * (ssd_config.page_data_size + ssd_config.page_spare_size) + page_buffer_spare_chunk;

		if(nil_request.OPC == PROGRAM)
		{
			printf("NIL: MAKE PROGRAM REQUEST PACKET (ID: %d).\n", nil_request.id);
			if(nand_dma_in(channel, lun, host_get_data_address(nil_request.id), host_get_spare_address(nil_request.id))==SUCCESS)
				result = nand_page_program(channel, lun, block, page);
		}
		else if(nil_request.OPC == READ)
		{
			printf("NIL: MAKE READ REQUEST PACKET (ID: %d).\n", nil_request.id);
			if(nand_page_read(channel, lun, block, page) == SUCCESS)
				result = nand_dma_out(channel, lun, host_get_data_address(nil_request.id), host_get_spare_address(nil_request.id));
		}
		else
		{
			printf("NIL: REQUEST OPC ERROR.\n");
		}

		// return value에 따라 nil_response 생성.
		printf("NIL: MAKE RESPONSE PACKET (ID: %d).\n", nil_request.id);
		nil_response.id = nil_request.id;
		if(result == SUCCESS)
		{
			nil_response.status = 0;
			printf("| [Hex viewer address]  data: 0x%x,  spare: 0x%x\n", hex_viewer_data, hex_viewer_spare);
		}
		else
		{
			nil_response.status = 1;
		}

		if(cqueue_enqueue(&nil_response_queue, &nil_response) == SUCCESS)
			printf("NIL: Enqueue the nil_response_packet in nil_response_queue.\n");
		else
			printf("NIL: Response Enqueue ERROR.\n");
	}
	else
	{
		printf("NIL: REQUEST/RESPONSE FAIL.\n");
	}
}






// nand 함수 순서 test를 위한 nil main() : test 3, test 4
void nil_main_test()
{
	int channel, lun, block, page;
	int result1, result2;
	if(cqueue_test(&nil_response_queue) != FULL && cqueue_test(&nil_request_queue) != EMPTY)
	{
		cqueue_dequeue(&nil_request_queue, &nil_request);

		channel = (nil_request.page_address & 0x3c000000) >> 26;
		lun = (nil_request.page_address & 0x03c00000) >> 22;
		block = (nil_request.page_address & 0x003ffe00) >> 9;
		page = nil_request.page_address & 0x000001ff;

		//4K Chunk Validity Vector ?
		if(nil_request.OPC == PROGRAM)
		{
			printf("NIL: MAKE PROGRAM REQUEST PACKET (ID: %d).\n", nil_request.id);
			printf("| channel: %d, lun: %d, block: %d, page: %d, 4k_chunk_validity_vector: %d\n", channel, lun, block, page, nil_request.fourk_chunk_validity_vector);

			result2 = nand_page_program(channel, lun, block, page);
			result1 = nand_dma_in(channel, lun, host_get_data_address(nil_request.id), host_get_spare_address(nil_request.id));
		}
		else if(nil_request.OPC == READ)
		{
			printf("NIL: MAKE READ REQUEST PACKET (ID: %d).\n", nil_request.id);
			printf("| channel: %d, lun: %d, block: %d, page: %d, 4k_chunk_validity_vector: %d\n", channel, lun, block, page, nil_request.fourk_chunk_validity_vector);

			result2 = nand_dma_out(channel, lun, host_get_data_address(nil_request.id), host_get_spare_address(nil_request.id));
			result1 = nand_page_read(channel, lun, block, page);
		}

		// return value에 따라 nil_response 생성.
		printf("NIL: MAKE RESPONSE PACKET (ID: %d).\n", nil_request.id);
		nil_response.id = nil_request.id;
		if(result1 == SUCCESS && result2 == SUCCESS)
			nil_response.status = 0;
		else
			nil_response.status = 1;

		if(cqueue_enqueue(&nil_response_queue, &nil_response) == SUCCESS)
			printf("NIL: Enqueue the nil_response_packet in nil_response_queue.\n");
		else
			printf("NIL: Response Enqueue ERROR.\n");
	}
	else
	{
		printf("NIL: RESPONSE FAIL.\n");
	}
}
