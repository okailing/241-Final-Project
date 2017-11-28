#include "waveparse.h"

#include <inttypes.h>
#include <stdlib.h>
long position=0;

//calculates the "instant energy" at a given sample location

struct Node{
    long energy;
    long samplenumber;
    struct Node *next;
    struct Node *prev;
    struct Node *head;
    struct Node *tail;
};


long instantEnergy(WAVE *file, int sample){
	long energy = 0;
	short bytesPerSample = file->bytesPerSample;
	short numChannels = file->numChannels;
	char *data = file->data;

	for(int i = sample; i < sample + 1024*bytesPerSample*numChannels; i+=bytesPerSample){
		if(bytesPerSample == 1){
			int8_t val = (int8_t)*(data+i);
			val = val * val;
			energy += val;
		}	
		if(bytesPerSample == 2){
			int16_t val = (int16_t)*(data+i);
			val = val * val;
			energy += val;
		}
		if(bytesPerSample == 3){
			printf("I haven't figured out how to do this\n");
			exit(1);
		}
	}	

	return energy;
}

struct Node *add(struct Node *list, struct Node *tmp){
    tmp->prev = list->tail->prev;
    list->tail->prev->next=tmp; 
    tmp->next = list->tail;
    list->tail->prev = tmp;
    return list;
}

void add(struct Node *tail, struct Node *head, struct Node *newNode){
	tail->prev->next = newNode;
	newNode->prev = tail->prev;
	
	tail->prev = newNode;
	newNode->next = tail;// put new node at tail

	struct Node *toDelete = head->next;

	head->next->next->prev = head;
	head->next = head->next->next;

	free(toDelete);//remove new from tail
}

struct Node *init(){
	struct Node *tmphead=malloc(sizeof(struct Node));
	struct Node *tmptail=malloc(sizeof(struct Node));
	tmphead->head=tmphead;
	tmphead->tail=tmptail;
	tmphead->next=tmptail;
	tmphead->prev=NULL;
	tmptail->prev=tmphead;
	tmptail->next=NULL;
	tmphead->energy = -1;
	tmptail->energy = -1;
	tmphead->samplenumber = -1;
	tmptail->samplenumber = -1;
	return tmphead;
}

double average(struct Node *buffer){
    int sum=0;
    int nodes = 0;
    while(buffer->next->energy != -1){
	sum += buffer->next->energy;
	buffer=buffer->next;
	nodes++;
    }sum += buffer->tail->prev->energy;	//adds the very last node's energy, not added from the while loop
    nodes++;
    double avg = sum/nodes;
    return avg;
}

int main(int argc, char *argv[]){
	FILE *f = fopen(argv[1], "rb");

	WAVE *wave = readWave(f);
	struct Node *buffer = init();
	for(int i=0;i<44032;i=i+1024){
	    struct Node *tmp = malloc(sizeof(struct Node));
	    tmp->samplenumber = i;
	    tmp->energy=instantEnergy(wave,i);
	    buffer = add(buffer,tmp);
	}//average(buffer);
	//printf("%f\n", average(buffer));
	//printf("%ld\n", instantEnergy(wave, 0));
	printf("%ld\n", buffer->head->next->next->energy);
}
