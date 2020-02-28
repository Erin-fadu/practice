/*
 * nand.h
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */

#ifndef NAND_H_
#define NAND_H_

#define OVERWRITE	2
#define ERASED		3

#define LUNS_PER_CHANNEL	4
#define BLOCKS_PER_LUN		32
#define PAGE_BUFFER_SIZE 	18432

#define READY_DMA_IN		0
#define READY_PAGE_PROGRAM  1
#define READY_PAGE_READ		2
#define READY_DMA_OUT		4

typedef struct{
	int block_page_pointer[BLOCKS_PER_LUN];
	unsigned char page_buffer[PAGE_BUFFER_SIZE];
} _lun;

typedef struct{
	_lun lun[LUNS_PER_CHANNEL];
} nand_parameter;

int nand_init(void);
int nand_dma_in(int, int, int, void *, void *);
int nand_dma_out(int, int, int, int, void *, void *);
int nand_page_read(int, int, int, int);
int nand_page_program(int, int, int, int);
int nand_block_erase(int, int, int);

#endif /* NAND_H_ */
