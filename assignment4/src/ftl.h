/*
 * ftl.h
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */

#ifndef FTL_H_
#define FTL_H_

#include "list.h"
#include "nil.h"
#include "cqueue.h"
#include "main.h"
#include "nand.h"

#define OP_READ		0X42
#define OP_WRITE	0X41

#define MAX_MUS_PER_PAGE 		4
#define MAX_FTL_MERGE_ENTRIES   8
#define MAX_VALID_PAGE_COUNT  	257

#define UNWRITTEN_MPA	 		0xffffffff
#define MERGE_FULL				0xffffffff

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

typedef struct{
	int ch, lun, block, page;
	int erase_count, valid_page_count;
	struct list_head list;
} block;

typedef struct{
	int used;
	unsigned int id;
	unsigned int lba;
	unsigned int mpa;
} merge_entry;

typedef struct{
	int used;
	merge_entry entries[MAX_MUS_PER_PAGE];
} merge_entries;

typedef struct{
	merge_entries entries[MAX_FTL_MERGE_ENTRIES];
	int current_write_entry;
	int current_read_entry;
} _ftl_merge_manager;

int ftl_init(void);
void ftl_main(void);

int ftl_merge_entry_put(ftl_request_packet, int, int, int);
int ftl_make_request(int, int, int);
void ftl_valid_page_print(void);
void ftl_read_cant_merge(int, int, ftl_request_packet);
void ftl_read_unwritten(ftl_request_packet);
int ftl_make_response(int, int);

#endif /* FTL_H_ */
