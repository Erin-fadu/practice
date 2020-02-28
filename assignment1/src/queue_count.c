#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ITEM 8

#define ENQUEUE 1
#define DEQUEUE 2
#define TEST 3

int head;
int tail;
int buffer[MAX_ITEM];
int count;

void init(void);
int enqueue(int);
int dequeue(int *);
void print_queue(void);

int main()
{
	int num=0, item=0;
	int ran1=0, ran2=0;

	srand(time(NULL));
	init();

	while(1)
	{
		printf("1.Enqueue  2.dequeue  3.Test  4.Exit\nEnter the command: ");
		fflush(stdout);
		scanf("%d", &num);
		switch(num)
		{
		case ENQUEUE:
			printf("Enter the number: ");
			fflush(stdout);
			scanf("%d", &item);
			enqueue(item);
			break;
		case DEQUEUE:
			dequeue(&item);
			break;
		case TEST:
			while(1)
			{
				ran1 = rand()%4;
				ran2 = rand()%10;
				if(ran1==0||ran1==1||ran1==2)
				{
					if(enqueue(ran2)==0)
						continue;
					else
						break;
				}
				else if(ran1 == 3)
				{
					if(dequeue(&item)==0)
						continue;
					else
						break;
				}
			}
			break;
		default:
			return 0;
		}
	}
	return 0;
}

void init(void)
{
	head=0;
	tail=0;
	count=0;
	int i=0;
	for(i=0; i<MAX_ITEM; i++)
	{
		buffer[i]=0;
	}
}

int enqueue(int item)
{
	if(count==MAX_ITEM)
	{
		printf("ERROR: Queue is Full !\n");
		return 1;
	}
	else
	{
		buffer[tail] = item;
		tail = (tail+1)%MAX_ITEM;
		count++;
		print_queue();
		return 0;
	}
}

int dequeue(int *ret_item)
{
	if(count == 0)
	{
		printf("ERROR: Queue is Empty !\n");
		return 2;
	}
	else
	{
		*ret_item = buffer[head];
		buffer[head]=0;
		head= (head+1)%MAX_ITEM;
		count--;
		print_queue();
		printf("Dequeue item: %d", *ret_item);
		return 0;
	}
}

void print_queue(void)
{
	int i=0;
		printf("| ");
		for(i=0; i<MAX_ITEM; i++)
		{
			printf("%3d| ", buffer[i]);
		}
		printf("\n");
	//------------------------------------------
		printf("| ");
		for(i=0; i<MAX_ITEM; i++)
		{
			if(i==head)
				printf(" h | ");
			else
				printf("   | ");
		}
		printf("\n| ");
		for(i=0; i<MAX_ITEM; i++)
		{
			if(i==tail)
				printf(" t | ");
			else
				printf("   | ");
		}
	//------------------
		printf("\n=>");
		for(i=head; i!=tail+1; i++)
		{
			i = i%MAX_ITEM;
			printf("  %d", buffer[i]);
		}
		printf("\n\n");
}





