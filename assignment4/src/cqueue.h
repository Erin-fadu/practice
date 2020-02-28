/*
 * cqueue.h
 *
 *  Created on: 2020. 1. 29.
 *      Author: FD08
 */

#ifndef CQUEUE_H_
#define CQUEUE_H_

#define MAX_BUFFER_LEN	1024

#define SUCCESS	0
#define FAIL	1
#define FULL	2
#define EMPTY	3

typedef struct{
	int head;
	int tail;
	int count;
	int item_size;
	unsigned char buffer[MAX_BUFFER_LEN];
} cqueue;

int cqueue_init(cqueue *, int);
int cqueue_enqueue(cqueue *, void *);
int cqueue_dequeue(cqueue *, void *);

void cqueue_print(cqueue *);
int cqueue_test(cqueue *);

#endif /* CQUEUE_H_ */
