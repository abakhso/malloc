#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


typedef struct memHeader{ //information about memory block
	int size; //size of memory block
	int isFree; //is it free or somethin is allocated
	struct memHeader* next; //pointer to information of next memory block
	struct memHeader* previous; //pointer to information of previous memory block

}memHeader;

struct memHeader* first;
struct memHeader* last;
static int PAGESIZE = 4096;


struct memHeader* getFreeMemory(int size){
	struct memHeader *mem;
	struct memHeader * current;
	struct memHeader * maxSpace;
	for(current = first; current!=NULL; current=current->next){
		if(current->isFree == 0){
			if(maxSpace==NULL ||  current->size > maxSpace->size){
				maxSpace=current;
			}
		}
	}
	if(maxSpace==NULL) return NULL;
	if(maxSpace->size < size) return NULL;
	int Left = maxSpace->size - sizeof(memHeader) - size;
	if(Left>0){
		struct memHeader *newStruct;
		newStruct = (struct memHeader *)((char*)maxSpace+sizeof(struct memHeader)+size);
		newStruct->size=Left;
		newStruct->isFree=0;
		newStruct->previous=maxSpace;
		newStruct->next=maxSpace->next;
		maxSpace->size = size;
		maxSpace->isFree = 1;
		maxSpace->next=newStruct;
		mem=maxSpace;
		return mem;
	} else {
		maxSpace->isFree = 1;
		mem = maxSpace;
		return mem;
	}
}

void *mmalloc(int size){
	void * res;
	if(first==NULL){ //if it is first allocation of memory
		first = sbrk(sizeof(struct memHeader));
		first->isFree = 1;
		first->previous = NULL;
	//	first->next = NULL;
		int multiple = size/PAGESIZE+1;
		multiple = multiple * PAGESIZE;
		res = sbrk(multiple);
		if(multiple-size-sizeof(struct memHeader) > 0){
			last = (struct memHeader *)((char*)res + size);
			last->size=multiple-size-sizeof(struct memHeader);
			last->isFree=0;
			last->previous=first;
			first->next=last;
			first->size = size;
		} else {
			first->next =NULL;
			first->size = multiple;
			last=first;
		}
		return res;
	}else {
		struct memHeader *memoryInfo;
		memoryInfo = NULL;
		memoryInfo = getFreeMemory(size);
		if(memoryInfo!=0){
			return (char*)memoryInfo+sizeof(struct memHeader);
		} else {
			if(last->isFree==1){
				memoryInfo = sbrk(sizeof(struct memHeader));
				memoryInfo->size = size;
				memoryInfo->isFree=1;
				memoryInfo->previous = last;
				last->next = memoryInfo;
				last = memoryInfo;
				last->next=NULL;
				int multiple = size/PAGESIZE+1;
				multiple = multiple * PAGESIZE;
				res = sbrk(multiple);
				int newSize = multiple - size - sizeof(struct memHeader);
				if(newSize>0){
					struct memHeader* newStruct = (struct memHeader *)((char*)res+size);
					newStruct->isFree=0;
					newStruct->size=newSize;
					newStruct->next=NULL;
					newStruct->previous=last;
					last->next=newStruct;
					last=newStruct;
				}
			} else {
				int multiple = size/PAGESIZE+1;
				multiple = multiple * PAGESIZE;
				sbrk(multiple);

				res =(struct memHeader*)((char*)last+sizeof(struct memHeader));
				int oldSize = last->size;
				//printf("%ddradada\n",last->size );
				last->size=size; //axali daboloeba shevkmnat
				last->isFree=1;
				int newSize = oldSize+multiple - size - sizeof(struct memHeader);
				if(newSize > 0){
					memoryInfo = (struct memHeader*)((char*)last+sizeof(struct memHeader) + size);
					memoryInfo->size = newSize;
					memoryInfo->next = NULL;
					memoryInfo->previous = last;
					last->next=memoryInfo;
					last=memoryInfo;
				}
				struct memHeader* mes = (struct memHeader*)(char*)(res-sizeof(struct memHeader));

				//printf("%d-----------------%d\n",mes,res);
			}
		}
		return res;
	}

}



void decrease(){
	int size, multiple;
	if(last->isFree == 0 && last->size > PAGESIZE){
		multiple = last->size/PAGESIZE;
		multiple = multiple * PAGESIZE;
		if(multiple==last->size){
			last = last->previous;

			last->next=NULL;
			sbrk(-1*(size + sizeof(struct memHeader)));
		} else {
			last->size=last->size-multiple;
			sbrk(-1*multiple-1);
		}
		
	}

}

void ffree(void * memToFree){

	struct memHeader * tofree = (struct memHeader *)((char*)memToFree-sizeof(struct memHeader));
	if(tofree->isFree==0){
		printf("pointer is already free\n");
		return;
	}

	tofree->isFree=0;


	if(tofree->next!=NULL && tofree->next->isFree==0){
		tofree->size += tofree->next->size + sizeof(struct memHeader);
		tofree->isFree =0;
		if(tofree->next->next!= NULL){
			tofree->next = tofree->next->next;
			struct memHeader* nextStruct= tofree->next;
			nextStruct->previous = tofree;
		} else {
			tofree->next=NULL;
			last = tofree;
		}
	}


	if(tofree->previous!=NULL && tofree->previous->isFree==0){
		struct memHeader * previous = tofree->previous;
		previous->size += tofree->size + sizeof(struct memHeader);
		if(tofree->next!=NULL){
			previous->next = tofree->next;
			struct memHeader* nextStruct= tofree->next;
			nextStruct->previous= previous;
		} else {
			previous->next=NULL;
			last = previous;
		}
	}	
	decrease();
	

}

void meminfo(){
	printf("Memory information\n");
	struct memHeader * current;
	for(current=first; current!=NULL; current=current->next){
		if(current->isFree==0){
			printf("Memory--%p------Size--%d--------is Free\n",current, current->size);
		}else{
			printf("Memory--%p------Size--%d--------is Used\n",current, current->size);
		}
	}
}

int main(int argc,const char *argv[]){
	void * a = mmalloc(1);
	void * b= mmalloc(4080);
	//struct memHeader * tofree = (struct memHeader *)((char*)b-sizeof(struct memHeader));
	//printf("%d\n", b);
	//meminfo();
	//void * c = mmalloc(12500);
	ffree(b);
	ffree(a);
	//ffree(c);
	meminfo();
	return 0;
}