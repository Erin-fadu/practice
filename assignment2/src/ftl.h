/*
 * ftl.h
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */

#ifndef FTL_H_
#define FTL_H_

#define OP_READ 	0x5
#define OP_PROGRAM  0x2

typedef struct request{
	unsigned char id;
	unsigned char opcode;
	unsigned int address;
} request;

typedef struct{
	unsigned char id;
	unsigned char status;
} response;

#define RW_READY		0
#define RW_SUCCESS		1
#define NAND_SUCCESS 	2

int ftl_init(void);
int ftl_read(int);
int ftl_write(int);
void ftl_main(void);

#endif /* FTL_H_ */
