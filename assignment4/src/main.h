/*
 * main.h
 *
 *  Created on: 2020. 2. 7.
 *      Author: FD08
 */

#ifndef MAIN_H_
#define MAIN_H_


#define LBA_SIZE 			4096
#define SPARE_SIZE_PER_LBA  512
#define MAX_HOST_BUFFERS	32

typedef struct{
	int channel_count;
	int luns_per_channel;
	int blocks_per_lun;
	int pages_per_block;
	int page_data_size;
	int page_spare_size;
	long lba_count;
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
int ssd_config_init(void);
int main();

void host_write_request(int);
void host_read_request(int);
void host_response(void);

#endif /* MAIN_H_ */
