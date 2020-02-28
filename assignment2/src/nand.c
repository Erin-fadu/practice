#include <stdio.h>
#include "nand.h"
#include "ftl.h"
#include "cqueue.h"

extern request *p_request;
extern response *p_response;

extern cqueue request_queue, response_queue;

extern flag;

int nand_init(void)	// 초기화 하는 함수
{
	p_response->id = 0;
	p_response->status = 0;

	if(p_response->id !=0 || p_response->id != 0)
		return FAIL;

	return SUCCESS;
}

void nand_main(void) //	nand의 request_queue에서 request 한 개를 dequeue함
{
	request temp;
	printf(" [ FUNCTION: nand_main() ]\n");

	if(flag == RW_SUCCESS)
	{
		if(cqueue_dequeue(&request_queue, &temp) == EMPTY)
		{
			printf(" => DEQUEUE FAIL\n\n");
		}
		else
		{
			printf(" 	 < REQUEST QUEUE > \n");
			print_queue(&request_queue);
			printf("	  Dequeue item: %x%x%x\n\n\n", temp.id, temp.opcode, temp.address);

			//	request에 대한 response를 생성
			p_response->id = p_request->id;
			p_response->status = 0;

			// response_queue에 enqueue함
			cqueue_enqueue(&response_queue, p_response);
			printf(" 	 < RESPONSE QUEUE >\n");
			print_queue(&response_queue);

			flag = NAND_SUCCESS;
		}

	}
	else
		printf(" => NAND MAIN FAIL\n\n");
}
