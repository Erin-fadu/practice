/*
 * main.h
 *
 *  Created on: 2020. 1. 28.
 *      Author: FD08
 */

#ifndef MAIN_H_
#define MAIN_H_

#define LBA_SIZE 			4096
#define SPARE_SIZE_PER_LBA  512
#define MAX_HOST_BUFFERS	16

typedef struct{
	int channel_count;
	int luns_per_channel;
	int blocks_per_lun;
	int pages_per_block;
	int page_data_size;
	int page_spare_size;
} ssdconfig;

typedef struct{
	int used;
	unsigned char data[LBA_SIZE];
	unsigned char spare[SPARE_SIZE_PER_LBA];
} host_buffer;

typedef struct{
	int buffer_count;
	int num_used;
	host_buffer buffers[MAX_HOST_BUFFERS];
} host_buffer_manager;

short host_alloc_buffer(void);
int host_release_buffer(short);
void *host_get_data_address(short);
void *host_get_spare_address(short);
int init(void);
int str_to_int(FILE *, char *);
int main();

void host_buffer_print(host_buffer_manager *, short);
void host_flow_print(host_buffer_manager*);
void test_case_1(void);
void test_case_2(void);
void test_case_3(void);
void test_case_4(void);
void program_read_test(void);

#endif /* MAIN_H_ */
