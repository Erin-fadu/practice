/*
 * ftl_main() 에서는 ftl_response_queue를 dequeue하여 nil_request를 만들어 nil_request_queue에 enqueue하며,
 * nil_response_queue를 dequeue하여 ftl_response를 만들어 ftl_response_queue에 enqueue
*/

#include <stdio.h>
#include "ftl.h"
#include "cqueue.h"
#include "nil.h"

cqueue ftl_request_queue, ftl_response_queue;
extern cqueue nil_request_queue, nil_response_queue;

ftl_request_packet  ftl_request;
ftl_response_packet ftl_response;
extern nil_request_packet nil_request;

int ftl_init(void)
{
	printf("FTL: INITIAL\n");
	memset(&ftl_request, 0, sizeof(ftl_request_packet));
	memset(&ftl_response, 0, sizeof(ftl_response_packet));

	if(cqueue_init(&ftl_request_queue, sizeof(ftl_request_packet)) == FAIL)
		return FAIL;

	if(cqueue_init(&ftl_response_queue, sizeof(ftl_response_packet)) == FAIL)
		return FAIL;

	return SUCCESS;
}

void ftl_main(void)
{
	ftl_request_packet ftl_req_temp;
	nil_response_packet nil_res_temp;
	if(cqueue_test(&nil_request_queue)!=FULL && cqueue_test(&ftl_request_queue)!=EMPTY) // if nil_request_queue is not full
	{
		cqueue_dequeue(&ftl_request_queue, &ftl_req_temp); // dequeue a item from ftl_request_queue
		if(ftl_req_temp.opcode == OP_WRITE)
		{
			// 해당 nil_request를 생성하여 nil_request_queue에 enqueue
			printf("FTL: MAKE PROGRAM REQUEST PACKET (ID: %d).\n", ftl_req_temp.id);
			printf("| opcode: %d, id: %d, lba: %d\n", ftl_req_temp.opcode, ftl_req_temp.id, ftl_req_temp.lba);

			// nil request packet 생성
			nil_request.OPC = PROGRAM;
			nil_request.page_address = ftl_req_temp.lba / 4;
			nil_request.fourk_chunk_validity_vector = 1 << (ftl_req_temp.lba % 4);
			nil_request.id = ftl_req_temp.id;

			// Enqueue
			if(cqueue_enqueue(&nil_request_queue, &nil_request) == SUCCESS)
				printf("FTL: Enqueue the nil_request_packet in nil_request_queue.\n");
			else
				printf("FTL: Reqeust Enqueue ERROR.\n");
		}
		else if(ftl_req_temp.opcode == OP_READ)
		{
			// 해당 nil_request를 생성하여 nil_request_queue에 enqueue
			printf("FTL: MAKE READ REQUEST PACKET (ID: %d).\n", ftl_req_temp.id);
			printf("| opcode: %d, id: %d, lba: %d\n", ftl_req_temp.opcode, ftl_req_temp.id, ftl_req_temp.lba);

			// nil request packet 생성
			nil_request.OPC = READ;
			nil_request.page_address = ftl_req_temp.lba / 4;
			nil_request.fourk_chunk_validity_vector = 1 << (ftl_req_temp.lba % 4);
			nil_request.id = ftl_req_temp.id;

			// Enqueue
			if(cqueue_enqueue(&nil_request_queue, &nil_request) == SUCCESS)
				printf("FTL: Enqueue the nil_request_packet in nil_request_queue.\n");
			else
				printf("FTL: Request Enqueue ERROR.\n");
		}
		else
		{
			printf("FTL: REQUEST ERROR.\n");
		}
	}
	else
	{
		printf("FTL: REQUEST FAIL.\n");
	}

	//	ftl_response_queue가 not full이면, ftl_response_queue에서 response 한 개를 dequeue
	if(cqueue_test(&ftl_response_queue)!=FULL && cqueue_test(&nil_response_queue)!=EMPTY)
	{
		if(cqueue_dequeue(&nil_response_queue, &nil_res_temp) == SUCCESS)
		{
			//	nil_response를 보고, ftl_response를 생성하여 ftl_response_queue에 enqueue
			printf("FTL: MAKE RESPONSE PACKET (ID: %d).\n", nil_res_temp.id);
			ftl_response.status = nil_res_temp.status;
			ftl_response.id = nil_res_temp.id;
			if(cqueue_enqueue(&ftl_response_queue, &ftl_response) == SUCCESS)
				printf("FTL: Enqueue the ftl_reponse_packet in ftl_response_queue.\n");
			else
				printf("FTL: Response Enqueue ERROR.\n");
		}
	}
	else
	{
		printf("FTL: RESPONSE FAIL.\n");
	}
}
















