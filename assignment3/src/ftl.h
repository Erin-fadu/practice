/*
 * ftl.h
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */

#ifndef FTL_H_
#define FTL_H_

#define OP_READ		0X42
#define OP_WRITE	0X41

typedef struct{
	unsigned int opcode	: 8;
	unsigned int flags	: 8;
	unsigned int id		: 16;
	unsigned int lba	: 32;
	unsigned int 		: 8;
	unsigned int rsrvd	: 24;
	unsigned int		: 24;
	unsigned int dsm	: 8;
} ftl_request_packet;

typedef struct{
	unsigned int status : 8;
	unsigned int flags	: 8;
	unsigned int id		: 16;
} ftl_response_packet;

int ftl_init(void);
void ftl_main(void);

void ftl_request_print(ftl_request_packet *);

#endif /* FTL_H_ */
