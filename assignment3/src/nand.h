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

#define PAGE_READ_OK	0
#define PAGE_READ_NO	1
#define DMA_IN_OK		3
#define DMA_IN_NO		4

typedef struct{
	int block[BLOCKS_PER_LUN];
	unsigned char page_buffer[PAGE_BUFFER_SIZE];
} _lun;

typedef struct{
	_lun lun[LUNS_PER_CHANNEL];
} nand_parameter;

int page_buffer_data_chunk;
int page_buffer_spare_chunk;
int chunk;

int nand_init(void);
int nand_dma_in(int, int, void *, void *);
int nand_dma_out(int, int, void *, void *);
int nand_page_read(int, int, int, int);
int nand_page_program(int, int, int, int);
int nand_block_erase(int, int, int);

#endif /* NAND_H_ */
