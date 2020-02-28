#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "ftl.h"

cqueue ftl_request_queue, ftl_response_queue;
extern cqueue nil_request_queue, nil_response_queue;

ftl_request_packet  ftl_request;
ftl_response_packet ftl_response;
nil_response_packet nil_response;

extern ssdconfig ssd_config;

struct list_head valid_page_count[MAX_VALID_PAGE_COUNT];
block *block_list;
block *active_block;
_ftl_merge_manager ftl_merge_manager;
unsigned int *page_table;

int mu=0;

int ftl_init(void)
{
	int i=0, block_list_count;
	int ch_num=0, lun_num=0, block_num=0;
	nil_request_packet nil_req;
	printf("FTL: INITIAL\n");
	memset(&ftl_request, 0, sizeof(ftl_request_packet));
	memset(&ftl_response, 0, sizeof(ftl_response_packet));
	memset(&nil_response, 0, sizeof(nil_response_packet));

	if(cqueue_init(&ftl_request_queue, sizeof(ftl_request_packet)) == FAIL)
		return FAIL;
	if(cqueue_init(&ftl_response_queue, sizeof(ftl_response_packet)) == FAIL)
		return FAIL;

	// block list memory pool
	block_list_count = ssd_config.channel_count * ssd_config.luns_per_channel * ssd_config.blocks_per_lun;
	block_list = (block *)calloc(block_list_count, sizeof(block));
	// page table
	page_table = (unsigned int *)malloc(ssd_config.lba_count * sizeof(unsigned int));
	memset(page_table, UNWRITTEN_MPA, ssd_config.lba_count * sizeof(unsigned int));
	// ftl merge manager 초기화
	memset(&ftl_merge_manager, 0, sizeof(ftl_merge_manager));
	ftl_merge_manager.current_read_entry = MAX_FTL_MERGE_ENTRIES/2;

	// linked list로 block list 관리
	for(i=0; i<MAX_VALID_PAGE_COUNT; i++) // valid page count 값을 head로 지정
	{
		INIT_LIST_HEAD(&valid_page_count[i]);
	}
	for(i=0; i < block_list_count; i++) // block list ch, lun, block 관리
	{
		if(block_num == ssd_config.blocks_per_lun)
		{
			block_num = 0;
			lun_num++;
		}
		if(lun_num == ssd_config.luns_per_channel)
		{
			lun_num = 0;
			ch_num++;
		}
		block_list[i].block = block_num;
		block_list[i].lun = lun_num;
		block_list[i].ch = ch_num;
		block_list[i].page = 0;
		list_add_tail(&block_list[i].list, &valid_page_count[0]);
		block_num++;
	}

	active_block = list_entry(valid_page_count[0].next, block, list);  // active block 선정
	if(cqueue_test(&nil_request_queue)!=FULL)	// block erase
	{
		nil_req.OPC = ERASE;
		nil_req.page_address = (active_block->ch<<26) | (active_block->lun<<22) | (active_block->block<<9) ;
		nil_req.id = ERASE;
		if(cqueue_enqueue(&nil_request_queue, &nil_req) != SUCCESS)
			printf("FTL: Request Enqueue ERROR.\n");
	}
	nil_main();
	ftl_main();

	active_block->erase_count++;
	active_block->valid_page_count = MAX_VALID_PAGE_COUNT-1;
	list_move_tail(&active_block->list, &valid_page_count[MAX_VALID_PAGE_COUNT-1]);
	return SUCCESS;
}

void ftl_valid_page_print(void)
{
	block *new;
	struct list_head *p;
	printf("----------------------------------\n");
	for(int i=0; i<MAX_VALID_PAGE_COUNT; i++)
	{
		printf("(%d)", i);
		list_for_each(p, &valid_page_count[i])
		{
			new = list_entry(p, block, list);
			printf("[%d.%d.%d]", new->ch, new->lun, new->block);
		}
		printf("\n");
	}
	printf("----------------------------------\n");
}


int ftl_merge_entry_put(ftl_request_packet ftl_req, int index, int used_count, int mpa)
{
	if(ftl_merge_manager.entries[index].entries[used_count].used == 0)
	{
		ftl_merge_manager.entries[index].entries[used_count].used = 1;
		ftl_merge_manager.entries[index].entries[used_count].id = ftl_req.id;
		ftl_merge_manager.entries[index].entries[used_count].lba = ftl_req.lba;
		ftl_merge_manager.entries[index].entries[used_count].mpa = mpa;
		ftl_merge_manager.entries[index].used++;
		return SUCCESS;
	}
	else
	{
		printf("FTL: merge entries already in use.\n");

		ftl_response_packet ftl_res;
		if(cqueue_test(&ftl_response_queue)!=FULL)
		{
			ftl_res.id = ftl_req.id;
			ftl_res.status = 1;
			assert(cqueue_enqueue(&ftl_response_queue, &ftl_res) == SUCCESS);
			printf("FTL: REQUEST FAIL, make FAIL RESPONSE. (ID: %d)\n", ftl_req.id);
		}
		else
		{
			printf("FTL: FTL response queue is full\n");
		}

		return FAIL;
	}
}

// NIL REQUEST PACKET을 만드는 함수
int ftl_make_request(int opc, int entry_index, int page_address)
{
	nil_request_packet nil_req;
	int new_entry;
	// 현재 merge entry 출력
	printf("FTL: Merged entry print. (entry: %d)\n", entry_index);
	printf("|  id  |  lba  |    mpa    |\n");
	for(int i=0; i < MAX_MUS_PER_PAGE; i++)
	{
		printf("|  %2d  | %5d | %9d |\n", ftl_merge_manager.entries[entry_index].entries[i].id,
				ftl_merge_manager.entries[entry_index].entries[i].lba, ftl_merge_manager.entries[entry_index].entries[i].mpa);
	}
	// 새로운 merge entry 할당 , current index 갱신
	new_entry = MERGE_FULL;
	if(opc == PROGRAM && ftl_merge_manager.entries[entry_index].used == 4)
	{
		for(int i=0; i<MAX_FTL_MERGE_ENTRIES/2; i++)	// 0 ~ 4번 entry에서 탐색
		{
			if(ftl_merge_manager.entries[i].used == 0)
			{
				new_entry = i;
				break;
			}
		}
	}
	else if(opc == READ && ftl_merge_manager.entries[entry_index].used != 0)
	{
		for(int i=MAX_FTL_MERGE_ENTRIES/2; i<MAX_FTL_MERGE_ENTRIES; i++)  // 5 ~ 9 번 entry에서 탐색
		{
			if(ftl_merge_manager.entries[i].used == 0)
			{
				new_entry = i;
				break;
			}
		}
	}
	else
	{
		printf("FTL: Request error.\n");
		assert(0);
	}
	printf("FTL: make NIL REQUEST packet, and Enqueue. (entry: %d, ID:", entry_index);
	for(int i=0; i<ftl_merge_manager.entries[entry_index].used; i++)
		printf(" %d", ftl_merge_manager.entries[entry_index].entries[i].id);
	printf(")\n");
	// nil request packet 생성
	nil_req.OPC = opc;
	nil_req.page_address = page_address;
	nil_req.id = entry_index;
	assert(cqueue_enqueue(&nil_request_queue, &nil_req) == SUCCESS); // nil request queue에 Enqueue

	return new_entry;
}

int ftl_make_response(int opc, int merged_index)
{
	if(opc == OP_WRITE)
	{
		printf("FTL: Make WRITE RESPONSE packet, and Enqueue (ID:");
		if(ftl_merge_manager.current_write_entry == MERGE_FULL)	// entry가 모두 꽉 찬 상태였으면, 방금 비운 entry를 current entry로 지정.
			ftl_merge_manager.current_write_entry = merged_index;
	}
	else if(opc == OP_READ)
	{
		printf("FTL: MAKE READ RESPONSE PACKET, and Enqueue. (ID:");
		if(ftl_merge_manager.current_read_entry == MERGE_FULL)
			ftl_merge_manager.current_read_entry = merged_index;
	}

	for(int i=0; i < ftl_merge_manager.entries[merged_index].used; i++) // response packet 만들어서 enqueue
	{
		printf(" %d", ftl_merge_manager.entries[merged_index].entries[i].id);
		ftl_response.status = nil_response.status;
		ftl_response.id = ftl_merge_manager.entries[merged_index].entries[i].id;
		assert(cqueue_enqueue(&ftl_response_queue, &ftl_response) == SUCCESS);
	}

	printf(")\n");
	memset(&ftl_merge_manager.entries[merged_index], 0, sizeof(ftl_merge_manager.entries[merged_index])); // entry 비우기

	return SUCCESS;
}


void ftl_read_cant_merge(int entry_index, int mpa, ftl_request_packet ftl_req) // merge 되었던 entry를 nil로 보내고 새로 들어온 req를 새롭게 merge
{
	int prev_page_address, new_index;
	// entry의 page address를 받아와서 nil에 넘겨줌
	prev_page_address = ftl_merge_manager.entries[entry_index].entries[0].mpa >> 2;
	ftl_merge_manager.current_read_entry = ftl_make_request(READ, entry_index, prev_page_address);
	new_index = ftl_merge_manager.current_read_entry;

	// 현재 request를 merge entry에 넣음
	if(ftl_merge_entry_put(ftl_req, new_index, ftl_merge_manager.entries[new_index].used, mpa)==SUCCESS)
		printf("FTL: READ REQUEST is newly Merged. (ID: %d, entry: %d)\n", ftl_req.id, new_index);
}


void ftl_direct_enqueue_ftl_response_queue(int isSuccess, ftl_request_packet ftl_req)
{
	ftl_response_packet ftl_res;
	if(cqueue_test(&ftl_response_queue)!=FULL)
	{
		ftl_res.id = ftl_req.id;
		if(isSuccess == SUCCESS)
		{
			ftl_res.status = 0;
			printf("FTL: PAGE TABLE is UNWRITTEN, make Response. (ID: %d)\n", ftl_req.id);
		}
		else
		{
			ftl_res.status = 1;
			printf("FTL: Error. Return FAIL. (ID: %d)\n", ftl_req.id);
		}
		assert(cqueue_enqueue(&ftl_response_queue, &ftl_res) == SUCCESS);

	}
	else
	{
		printf("FTL: FTL response queue is full\n");
	}
}

void ftl_write_request_merge(block *active_block)
{
	unsigned int page_address, mpa, entry_index, entry_used_count;
	if(mu > 3) mu = 0;
	page_address = (active_block->ch<<26) | (active_block->lun<<22) | (active_block->block<<9) | active_block->page;
	mpa = (page_address << 2) + mu;
	// 현재 사용중이던 entry의 빈공간에 값을 넣음.
	entry_index = ftl_merge_manager.current_write_entry;
	entry_used_count = ftl_merge_manager.entries[entry_index].used;
	if(ftl_merge_entry_put(ftl_request, entry_index, entry_used_count, mpa)==SUCCESS)
		printf("FTL: WRITE REQUEST is merged (id: %d, entry: %d)\n", ftl_request.id, entry_index);
	mu++;
	//  nil request packet 생성
	if(ftl_merge_manager.entries[entry_index].used == 4)
	{
		ftl_merge_manager.current_write_entry = ftl_make_request(PROGRAM, entry_index, page_address);
		active_block->page++;
	}
}



void ftl_main(void)
{
	printf("------------------------------------------------------\n");
	int entry_index, entry_used_count;
	int mu_used, page_address, merge_page_address, mpa;
	nil_request_packet nil_req;

// *********************************************************** FTL REQUEST *******************************************************
	// request를 하난 뽑기전에 Queue의 full/empty test와, merge가 꽉 찼는지 검사.
	if(cqueue_test(&nil_request_queue)!=FULL && cqueue_test(&ftl_request_queue)!=EMPTY && ftl_merge_manager.current_write_entry != MERGE_FULL && ftl_merge_manager.current_read_entry != MERGE_FULL)
	{
		assert(cqueue_dequeue(&ftl_request_queue, &ftl_request)==SUCCESS);
		// ************************************** WRITE REQUEST ***************************************
		if(ftl_request.opcode == OP_WRITE)
		{
			if(active_block->page == ssd_config.pages_per_block) // act block 끝가지 다 썼으면
			{
				if(list_empty(&valid_page_count[0])==0) // vpc 0에 block list가 남아있는지 검사
				{
					printf("FTL: new block will be used.\n");
					active_block = list_entry(valid_page_count[0].next, block, list);
					if(cqueue_test(&nil_request_queue)!=FULL)	// block erase
					{
						nil_req.OPC = ERASE;
						nil_req.page_address = (active_block->ch<<26) | (active_block->lun<<22) | (active_block->block<<9) ;
						nil_req.id = ERASE;
						assert(cqueue_enqueue(&nil_request_queue, &nil_req) == SUCCESS);
					}
					active_block->erase_count++;
					active_block->valid_page_count = MAX_VALID_PAGE_COUNT-1;
					list_move_tail(&active_block->list, &valid_page_count[MAX_VALID_PAGE_COUNT-1]);
					active_block->page=0;

					ftl_write_request_merge(active_block);
				}
				else
				{
					ftl_direct_enqueue_ftl_response_queue(FAIL, ftl_request);
//					assert(0);
				}
			}
			else
			{
				ftl_write_request_merge(active_block);
			}
		}
		 // *************************************** READ REQUEST ********************************************
		else if(ftl_request.opcode == OP_READ)
		{
			mpa = page_table[ftl_request.lba]; // page table look up. lba에 대한 mpa값을 구함.
			page_address = mpa >> 2;
			entry_index = ftl_merge_manager.current_read_entry;
			entry_used_count = ftl_merge_manager.entries[entry_index].used;

			if(mpa == UNWRITTEN_MPA)
			{
				ftl_direct_enqueue_ftl_response_queue(SUCCESS, ftl_request); 				// HOST BUFFER에 0 출력
			}
			else
			{
				if(entry_used_count == 0) // 현재 read merge중인 page가 없으면, mpa의 page를 merge중인 page로 선정
				{
					ftl_merge_entry_put(ftl_request, entry_index, entry_used_count, mpa);
					printf("FTL: READ REQUEST is newly Merged. (ID: %d, entry: %d)\n", ftl_request.id, entry_index);
				}
				else
				{
					// merge 가능한지 구함
					merge_page_address = ftl_merge_manager.entries[entry_index].entries[0].mpa >> 2; // merge entry 안의 있는 mpa의 address 구함
					if(page_address == merge_page_address)
					{
						for(int i=0; ftl_merge_manager.entries[entry_index].entries[i].used==1; i++)
						{
							// mu #에 대한 used값이 0이어야 함
							if((ftl_merge_manager.entries[entry_index].entries[i].mpa & 0x00000003) == (mpa & 0x00000003))
							{
								mu_used = FAIL;
								break;
							}
							else
							{
								mu_used = SUCCESS;
								continue;
							}
						}

						if (mu_used == SUCCESS) // page address가 같고 mu#에 대한 used 값이 0인지 검사.
						{
							// read request 하나를  merge
							ftl_merge_entry_put(ftl_request, entry_index, entry_used_count, mpa);
							printf("FTL: READ REQUEST is merged. (id: %d, entry: %d)\n", ftl_request.id, entry_index);

							// merge entry가 가득 차면 수행
							if(ftl_merge_manager.entries[entry_index].used == 4)
							{
								ftl_merge_manager.current_read_entry = ftl_make_request(READ, entry_index, page_address);
							}
						}
						else
						{
							ftl_read_cant_merge(entry_index, mpa, ftl_request);
						}

					}
					else
					{
						ftl_read_cant_merge(entry_index, mpa, ftl_request);
					}
				}
			}

		}
		else
		{
			printf("FTL: REQUEST ERROR.\n");
			assert(0);
		}
	}
	else
	{
		printf("FTL: REQUEST FAIL. Wait for next FTL_MAIN.\n");;
	}


// *********************************************************** FTL RESPONSE *******************************************************
	int merged_index;
	int channel, lun, block, block_idx;
	//	ftl_response_queue가 not full이면, ftl_response_queue에서 response 한 개를 dequeue
	if(cqueue_test(&ftl_response_queue)!=FULL && cqueue_test(&nil_response_queue)!=EMPTY)
	{
		if(cqueue_dequeue(&nil_response_queue, &nil_response) == SUCCESS)
		{
			if(nil_response.id >= 0 && nil_response.id < MAX_FTL_MERGE_ENTRIES/2) // ************* WRITE RESPONSE ****************
			{
				if(nil_response.status == 0) // program이 성공했을때만 page table 갱신
				{
					merged_index = nil_response.id;
					for(int i=0; i<MAX_MUS_PER_PAGE; i++)
					{
						mpa = page_table[ftl_merge_manager.entries[merged_index].entries[i].lba];
						if(mpa != UNWRITTEN_MPA) // page table[lba]가 이미 쓰여진 것이면, 해당 block을 찾아서 vpc 를 빼주고 head에 매달아 줌
						{
							channel = (mpa & 0xf0000000) >> 28;
							lun = (mpa & 0x0f000000) >> 24;
							block = (mpa & 0x00fff800) >> 11;
							block_idx = ssd_config.blocks_per_lun*(channel*ssd_config.luns_per_channel + lun) + block; // 32(ch*4+lun)+blk
							block_list[block_idx].valid_page_count--;
							list_move_tail(&block_list[block_idx].list, &valid_page_count[block_list[block_idx].valid_page_count]);
							printf("| Valid Page Count(ch%d/lun%d/block%d) is %d\n", channel, lun, block, block_list[block_idx].valid_page_count);
						}
						page_table[ftl_merge_manager.entries[merged_index].entries[i].lba] = ftl_merge_manager.entries[merged_index].entries[i].mpa;
					}
					printf("FTL: PAGE TABLE is Updated.(entry %d)\n", merged_index);
				}
				ftl_make_response(OP_WRITE, nil_response.id); // response packet 만들어서 enqueue
			}
			else if(nil_response.id >= MAX_FTL_MERGE_ENTRIES/2 && nil_response.id < MAX_FTL_MERGE_ENTRIES) // ********* READ RESPONSE ********
			{
				ftl_make_response(OP_READ, nil_response.id);  // response packet 만들어서 enqueue
			}
			else if(nil_response.id == ERASE) // ********************************* ERASE *********************************
			{
				printf("FTL: BLOCK ERASE SUCCESS.\n");
			}
			else
			{
				printf("FTL: RESPONSE ID ERROR.\n");
				assert(0);
			}
		}
		else
		{
			printf("FTL: RESPONSE dequeue FAIL.\n");
		}
	}
	else
	{
		printf("FTL: RESPONSE FAIL. Wait for next FTL_MAIN.\n");
	}
}



