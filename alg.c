#include "waveparse.h"

#include <inttypes.h>
#include <stdlib.h>
long position=0;

//calculates the "instant energy" at a given sample location

struct Node{
    long energy;
    long sampleNumber;
    struct Node *next;
    struct Node *prev;
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

//struct Node *add(struct Node *list, struct Node *tmp){
//    tmp->prev = list->tail->prev;
//    list->tail->prev->next=tmp; 
//    tmp->next = list->tail;
//    list->tail->prev = tmp;
//    return list;
//}

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

void initAdd(struct Node *tail, struct Node *newNode){
	tail->prev->next = newNode;
	newNode->prev = tail->prev;
	
	tail->prev = newNode;
	newNode->next = tail;// put new node at tail	
}

struct Node *init(){
	struct Node *new = malloc(sizeof(struct Node));
	new->energy = -1;
	new->sampleNumber = -1;
	new->next = NULL;
	new->prev = NULL;

	return new;
}

double average(struct Node *head){
	long total = 0;
	int count = 0;
	struct Node *curr = head->next;

	while(curr->sampleNumber != -1){
		total += curr->energy;
		count += 1;

		curr = curr->next;
	}

	return ((double) total) / count;
}

int main(int argc, char *argv[]){
	FILE *f = fopen(argv[1], "rb");

	WAVE *wave = readWave(f);

	struct Node *head = init();
	struct Node *tail = init();
	head->next = tail;
	tail->prev = head;

	for(int i=0;i<wave->sampleRate;i=i+1024){
		struct Node *tmp = init();
	    tmp->sampleNumber = i;
	    tmp->energy=instantEnergy(wave,i);
		initAdd(tail, tmp);    
	}
	
	printf("%f\n", average(head));


	fclose(f);
	free(wave->data);
	free(wave);

	struct Node *curr = head->next;
	while(curr->energy != -1){
		struct Node *next = curr->next;
		free(curr);
		curr = next;
	}
	free(head);
	free(tail);
}
