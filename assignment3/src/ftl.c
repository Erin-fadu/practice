/*
 * ftl_main() ������ ftl_response_queue�� dequeue�Ͽ� nil_request�� ����� nil_request_queue�� enqueue�ϸ�,
 * nil_response_queue�� dequeue�Ͽ� ftl_response�� ����� ftl_response_queue�� enqueue
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
			// �ش� nil_request�� �����Ͽ� nil_request_queue�� enqueue
			printf("FTL: MAKE PROGRAM REQUEST PACKET (ID: %d).\n", ftl_req_temp.id);
			printf("| opcode: %d, id: %d, lba: %d\n", ftl_req_temp.opcode, ftl_req_temp.id, ftl_req_temp.lba);

			// nil request packet ����
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
			// �ش� nil_request�� �����Ͽ� nil_request_queue�� enqueue
			printf("FTL: MAKE READ REQUEST PACKET (ID: %d).\n", ftl_req_temp.id);
			printf("| opcode: %d, id: %d, lba: %d\n", ftl_req_temp.opcode, ftl_req_temp.id, ftl_req_temp.lba);

			// nil request packet ����
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

	//	ftl_response_queue�� not full�̸�, ftl_response_queue���� response �� ���� dequeue
	if(cqueue_test(&ftl_response_queue)!=FULL && cqueue_test(&nil_response_queue)!=EMPTY)
	{
		if(cqueue_dequeue(&nil_response_queue, &nil_res_temp) == SUCCESS)
		{
			//	nil_response�� ����, ftl_response�� �����Ͽ� ftl_response_queue�� enqueue
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
















