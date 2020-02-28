#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define MAX_ITEM 8

int head;
int tail;
int buffer[MAX_ITEM];
int count;

void init(void);		// �ʱ�ȭ �ϴ� �Լ�
int enqueue(int);		// Circular Queue�� item�� �ִ� �Լ�
int dequeue(int *);		// Circular Queue���� item�� �������� �Լ�
void printQueue(void);	// Circular Queue�� ����ϴ� �Լ�

int main()
{
	int num=0;
	int item=0;

	int ran1, ran2;
	srand(time(NULL));

	init();
	while(1)
	{
		printf("1.Test   2. Enqueue   3. Dequeue   4. Exit\n Enter the test number: ");
		fflush(stdout);
		scanf("%d", &num);
		if(num==1) // full test
		{
			while(1)
			{
				ran1 = rand()%4;
				ran2 = rand()%10;
				if(ran1==0||ran1==1||ran1==2) 	// 3/4 enqueue
				{
					if(enqueue(ran2)==0)
						continue;
					else
						break;
				}
				else if(ran1 == 3)				// 1/4 dequeue
				{
					if(dequeue(&item)==0)
						continue;
					else
						break;
				}
			}

		}
		else if(num==2) // Enqueue
		{
			printf("-> Enqueue: ");
			fflush(stdout);
			scanf("%d", &item);
			enqueue(item);
		}
		else if(num==3)  // Dequeue
		{
			dequeue(&item);
		}
		else
			return 0;
	}
}

void init(void)
{
	tail=0;
	head=0;
	count=0;
	int i=0;
	for(i=0; i<MAX_ITEM; i++)
	{
		buffer[i]=0;
	}
}

int enqueue(int item)
{
	if(count == MAX_ITEM-1) // �� �̻� item�� enqueue�� �� ���� ����
	{
		printf("\n=> Circular Queue is Full!\n\n");
		return 1;
	}
	else // item�� enqueue�� �� �ִ� ����
	{
		tail = (tail+1)%MAX_ITEM;	// tail�� �����͸� �Ű���
		buffer[tail] = item;
		count++;
		printQueue();
		return 0;
	}
}

int dequeue(int *ret_item)
{
	if(count == 0) // item�� queue�� �� ���� ���� ����
	{
		printf("\n=> Circular Queue is Empty!\n\n");
		return 2;
	}
	else 	// item�� dequeue�� �� �ִ� ����
	{
		head = (head+1)%MAX_ITEM;	// head�� �����͸� �Ű���
		*ret_item = buffer[head];
		buffer[head] = 0;
		count--;
		printQueue();
		printf("Dequeue item: %d\n\n", *ret_item);
		return 0;
	}
}

void printQueue()
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
//-----------------------------------------
	printf("\n=> ");
	for(i=head+1; i!=tail+1; i++)
	{
		i = i%MAX_ITEM;
		printf("  %d", buffer[i]);
	}
	printf("\n\n");
}
