#include <stdio.h>
#include "ftl.h"
#include "cqueue.h"

request request1;
request *p_request = &request1;
response response1;
response *p_response = &response1;

cqueue request_queue, response_queue;

int flag = RW_READY;

int ftl_init(void) //초기화 하는 함수
{
	p_request->id = 0;
	p_request->opcode= 0;
	p_request->address = 0;
	cqueue_init(&request_queue, sizeof(request));
	cqueue_init(&response_queue, sizeof(response));

	if(p_request->id != 0 || p_request->opcode != 0 || p_request->address)
		return FAIL;

	return SUCCESS;
}

int ftl_read(int address) // nand의 request_queue에 read request를 enqueue하는 함수
{
	printf(" [ FUNCTION: ftl_read() ]\n");
	if(flag == RW_READY)
	{
		// request 만듦
		p_request->opcode = OP_READ;
		p_request->address = address;

		if(cqueue_enqueue(&request_queue, p_request) == SUCCESS)
		{
			printf(" 	 < READ REQEUSET QUEUE >\n");
			print_queue(&request_queue);

			p_request->id++;
			flag = RW_SUCCESS;
			return SUCCESS;
		}
		else
			return FAIL;
	}
	else
	{
		printf(" => FTL READ FAIL\n\n");
		return FAIL;
	}

}


int ftl_write(int address)	// nand의 request_queue에 program request를 enqueue하는 함수
{
	printf(" [ FUNCTION: ftl_write() ]\n");

	if(flag == RW_READY)
	{
		// request 만듦
		p_request->opcode = OP_PROGRAM;
		p_request->address = address;

		if(cqueue_enqueue(&request_queue, p_request) == SUCCESS)
		{
			printf(" 	 < WRITE REQUEST QUEUE > \n");
			print_queue(&request_queue);

			p_request->id++;
			flag = RW_SUCCESS;
			return SUCCESS;
		}
		else
			return FAIL;
	}
	else
	{
		printf(" => FTL WRITE FAIL\n\n");
				return FAIL;
	}
}

void ftl_main(void)
{
	//	nand의 response_queue에서 response 한 개를 dequeue
	//	response_queue가 empty가 아니었다면, dequeue한 response를 화면에 출력
	response temp;
	printf(" [ FUNCTION: ftl_main() ]\n");

	if(flag == NAND_SUCCESS)
	{
		if(cqueue_dequeue(&response_queue, &temp) == EMPTY)
		{
			printf(" => DEQUEUE FAIL\n\n");
		}
		else
		{
			printf(" 	 < RESPONSE QUEUE > \n");
			print_queue(&response_queue);
			printf("	  Dequeue item: %x %x\n\n", temp.id, temp.status);
			flag = RW_READY;
			printf("-----------------------------------------------------\n");
		}
	}
	else
		printf(" => FTL MAIN FAIL\n\n");
}
