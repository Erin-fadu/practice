/*
 * nil.h
 *
 *  Created on: 2020. 2. 10.
 *      Author: FD08
 */

#ifndef NIL_H_
#define NIL_H_

#define READ 	0X05
#define PROGRAM 0X02
#define ERASE	0x22

typedef struct{
	unsigned int OPC						: 8;
	unsigned int SLC						: 1;
	unsigned int no_scrambling				: 1;
	unsigned int read_retry_mode 			: 2;
	unsigned int do_not_drop				: 1;
	unsigned int fused 						: 1;
	unsigned int rsrvd 						: 2;
	unsigned int id							: 16;
	unsigned int page_address 				: 32;
	unsigned int reserved					: 32;
	unsigned int fourk_chunk_validity_vector: 8;
	unsigned int fault_injection_vector 	: 8;
	unsigned int 							: 8;
	unsigned int p_id						: 8;
} nil_request_packet;

typedef struct{
	unsigned int status : 8;
	unsigned int flags	: 8;
	unsigned int id		: 16;
} nil_response_packet;

int nil_init(void);
void nil_main(void);
void nil_main_test(void);

#endif /* NIL_H_ */
