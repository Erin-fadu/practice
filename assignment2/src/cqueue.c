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
	if(q == NULL)	// ������ NULL �˻�
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

int cqueue_enqueue(cqueue *q, void *p_item)					// p_Item: circular queue�� ���� item�� address
{
	if(q == NULL || p_item == NULL)											// ������ NULL test
		return FAIL;
	else if(q->count >= MAX_BUFFER_LEN)						// FULL test
	{
		printf("Circular Queue is Full !\n");
		return FULL;
	}
	else
	{
		memcpy(q->buffer + q->tail , p_item, q->item_size); // buffer[tail]���� item copy
		q->tail = (q->tail + q->item_size)%MAX_BUFFER_LEN; 	// tail�� item_size��ŭ ����
		q->count += q->item_size;							// count�� item_size��ŭ ����
		return SUCCESS;
	}
}

int cqueue_dequeue(cqueue *q, void *p_ret_item) 	// p_ret_item: item�� ���� address
{
	int i=0, temp=0;;
	if(q == NULL || p_ret_item == NULL)									// ������ NULL test
		return FAIL;
	else if(q->count <= 0)
	{
		printf("Circular Queue is Empty !\n");		// EMPTY test
		return EMPTY;
	}
	else
	{
		memcpy(p_ret_item, q->buffer + q->head, q->item_size);   // buffer[head]�� ���� dequeue
		temp = q->head;
		for(i=0; i<q->item_size; i++)
		{
			temp = temp%MAX_BUFFER_LEN;
			q->buffer[temp++] = 0;
		}
		q->head = (q->head + q->item_size)%MAX_BUFFER_LEN;			// head ������ item_size��ŭ ����
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
		for(i=0; i<q->count; i++)				// ���Ŀ� circular ����� ����� �� �ٲ�����.
		{
			printf("  %x", q->buffer[(temp++)%MAX_BUFFER_LEN]);
		}
	}
	else
	{
		for(i=q->head; i!=q->tail; i++)			// ���Ŀ� circular ����� ����� �� �ٲ�����.
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
