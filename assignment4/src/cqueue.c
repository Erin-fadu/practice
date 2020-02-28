/*
 * cqueue.c
 *
 *  Created on: 2020. 1. 29.
 *      Author: FD08
 */
#include <stdio.h>
#include <string.h>
#include "cqueue.h"


int cqueue_init(cqueue *q, int item_size)
{
	if(q == NULL)
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

int cqueue_enqueue(cqueue *q, void *p_item)
{
	if(q == NULL || p_item == NULL)
	{
		return FAIL;
	}
	else if(q->count >= MAX_BUFFER_LEN)
	{
		printf("Circular Queue is FULL !\n");
		return FULL;
	}
	else
	{
		memcpy(q->buffer + q->tail, p_item, q->item_size);
		q->tail = (q->tail + q->item_size)%MAX_BUFFER_LEN;
		q->count += q->item_size;
		return SUCCESS;
	}
}

int cqueue_dequeue(cqueue *q, void *p_ret_item)
{
	int i=0, temp=0;;
	if(q == NULL || p_ret_item == NULL)
		return FAIL;
	else if(q->count <= 0)
	{
		printf("Circular Queue is Empty !\n");		// EMPTY test
		return EMPTY;
	}
	else
	{
		memcpy(p_ret_item, q->buffer + q->head, q->item_size);
		temp = q->head;
		for(i=0; i<q->item_size; i++)
		{
			temp = temp%MAX_BUFFER_LEN;
			q->buffer[temp++] = 0;
		}
		q->head = (q->head + q->item_size)%MAX_BUFFER_LEN;
		q->count = q->count - q->item_size;
		return SUCCESS;
	}
}

void cqueue_print(cqueue *q)
{
	int i=0;
	printf("=>");
	if((q->head == q->tail) && (q->count == MAX_BUFFER_LEN))
	{
		int temp = q->head;
		for(i=0; i<q->count; i++)
		{
			printf(" %x", q->buffer[(temp++)%MAX_BUFFER_LEN]);
		}
	}
	else
	{
		for(i=q->head; i!=q->tail; i++)
		{
			if(i==8)
				break;
			else
			{
				i=i%MAX_BUFFER_LEN;
				printf(" %x", q->buffer[i]);
			}
		}
	}
	printf("\n\n");
}

int cqueue_test(cqueue *q)
{
	if(MAX_BUFFER_LEN%q->item_size == 0) // buffer 길이와 item size의 align이 맞는지 검사
	{
		if(q->count >= MAX_BUFFER_LEN)
			return FULL;
		else if(q->count <= 0)
			return EMPTY;
		else
			return SUCCESS;
	}
	else
		return FAIL;
}
















