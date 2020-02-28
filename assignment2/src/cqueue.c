/*
 * cqueue.c
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */
#include <stdio.h>
#include "cqueue.h"


int cqueue_init(cqueue *q, int item_size)
{
	if(q == NULL)	// 포인터 NULL 검사
		return FAIL;
	else
	{
		q->head = 0;
		q->tail = 0;
		q->count = 0;
		q->item_size = item_size;
		int i=0;
		for(i=0; i<MAX_BUFFER_LEN; i++)
			q->buffer[i] = 0;

		return SUCCESS;
	}
}

int cqueue_enqueue(cqueue *q, void *p_item)					// p_Item: circular queue에 넣을 item의 address
{
	if(q == NULL || p_item == NULL)											// 포인터 NULL test
		return FAIL;
	else if(q->count >= MAX_BUFFER_LEN)						// FULL test
	{
		printf("Circular Queue is Full !\n");
		return FULL;
	}
	else
	{
		memcpy(q->buffer + q->tail , p_item, q->item_size); // buffer[tail]부터 item copy
		q->tail = (q->tail + q->item_size)%MAX_BUFFER_LEN; 	// tail을 item_size만큼 증가
		q->count += q->item_size;							// count를 item_size만큼 증가
		return SUCCESS;
	}
}

int cqueue_dequeue(cqueue *q, void *p_ret_item) 	// p_ret_item: item을 받을 address
{
	int i=0, temp=0;;
	if(q == NULL || p_ret_item == NULL)									// 포인터 NULL test
		return FAIL;
	else if(q->count <= 0)
	{
		printf("Circular Queue is Empty !\n");		// EMPTY test
		return EMPTY;
	}
	else
	{
		memcpy(p_ret_item, q->buffer + q->head, q->item_size);   // buffer[head]의 값을 dequeue
		temp = q->head;
		for(i=0; i<q->item_size; i++)
		{
			temp = temp%MAX_BUFFER_LEN;
			q->buffer[temp++] = 0;
		}
		q->head = (q->head + q->item_size)%MAX_BUFFER_LEN;			// head 포인터 item_size만큼 증가
		q->count = q->count - q->item_size;									// count-1
		return SUCCESS;
	}
}

void print_queue(cqueue *q)
{
	printf("-----------------------------------------\n|");
	int i=0;
	for(i=0; i<MAX_BUFFER_LEN; i++)
	{
		printf("%3x |", q->buffer[i]);
	}
	printf("\n| ");
	for(i=0; i<MAX_BUFFER_LEN; i++)
	{
		if(i == q->head)
			printf(" h | ");
		else
			printf("   | ");
	}
	printf("\n| ");
	for(i=0; i<MAX_BUFFER_LEN; i++)
	{
		if(i == q->tail)
			printf(" t | ");
		else
			printf("   | ");
	}
	printf("\n-----------------------------------------\n => ");

	if((q->head == q->tail) && (q->count == MAX_BUFFER_LEN))
	{
		int temp = q->head;
		for(i=0; i<q->count; i++)				// 추후에 circular 기능을 써야할 때 바껴야함.
		{
			printf("  %x", q->buffer[(temp++)%MAX_BUFFER_LEN]);
		}
	}
	else
	{
		for(i=q->head; i!=q->tail; i++)			// 추후에 circular 기능을 써야할 때 바껴야함.
		{
			if(i==8)
				break;
			else
			{
				i=i%MAX_BUFFER_LEN;
				printf("  %x", q->buffer[i]);
			}
		}
	}
	printf("\n\n");
}
