#include "waveparse.h"

#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
long position=0;

#define MIN(A,B) ((A)<(B)?(A):(B))

//Node struct to store 1024 sample chunk in a doubly linked list
struct Node{
    long energy;
    long sampleNumber;
    struct Node *next;
    struct Node *prev;
};

//calculates the sum of energies in a 1024 sample chunk
long instantEnergy(WAVE *file, int sample){
	long energy = 0;
	short bytesPerSample = file->bytesPerSample;
	short numChannels = file->numChannels;
	char *data = file->data;

	for(int i = sample; i < MIN(file->dataSize, sample + 1024*bytesPerSample*numChannels); i+=bytesPerSample){
		if(bytesPerSample == 1){
			int8_t val = *((int8_t *) (data+i));
			val = val * val;
			energy += val;
		}	
		else if(bytesPerSample == 2){
			int16_t val = *((int16_t *) (data+i));
			val = val * val;
			energy += val;
		}
		else if(bytesPerSample == 3){ // I think this should work
			int val = 0;
			char  val_0 = *(data+i);
			char  val_1 = *(data + i + 1);
			char  val_2 = *(data + i + 2);
			val = val | (val_0 << 16);
			val = val | (val_1 << 8);
			val = val | val_2;
			if(val_0 & 0x80){
				val = val | (0xff << 24);
			}	
			val = val*val;
			return val;
		}
		else{
			printf("I don't think this exists");
			exit(1);
		}
	}	

	return energy;
}

//Adds a new node to the history buffer
//Works like queue - first in, first out
//deletes elements when adding
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

//Like add, but doesn't delete from list
void initAdd(struct Node *tail, struct Node *newNode){
	tail->prev->next = newNode;
	newNode->prev = tail->prev;
	
	tail->prev = newNode;
	newNode->next = tail;// put new node at tail	
}

//initializes a new node
struct Node *init(){
	struct Node *new = malloc(sizeof(struct Node));
	new->energy = -1;
	new->sampleNumber = -1;
	new->next = NULL;
	new->prev = NULL;

	return new;
}

//returns a deep copy of the node
struct Node *copy(struct Node *node){
	struct Node *new= init();
	new->energy = node->energy;
	new->sampleNumber = node->sampleNumber;
	return new;
}

//calculates the average energy of the history buffer
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

//calculates the sensitivity mutliplier based on the history buffer
double retc(struct Node *head){
    double sum0 = 0;
	int n = 0;
    struct Node *token = head;
    token = token->next;
    double avg = average(head);
    while (token->energy!=-1){
		sum0 += (token->energy - avg) * (token->energy - avg);
		token = token->next;
		n += 1;
    }
    double var = sum0/n;

    double c = (-0.0025714*var)+1.5142857;
    return c;
}

//used to conviniently free memory in a linked list
void freeUp(struct Node *head){
	struct Node *curr = head->next;
	
	while(curr->sampleNumber != -1){
		struct Node *nxt = curr->next;
		free(curr);
		curr=nxt;
	}	

	free(head);
	free(curr);
}

//finds the potential beats in a wave file
struct Node *findBeats(WAVE *wave){
	struct Node *headBuffer = init(); // creates buffer ist
	struct Node *tailBuffer = init();

	headBuffer->next = tailBuffer;
	tailBuffer->prev = headBuffer;

	int numChannels = wave->numChannels;
	int bytesPerSample = wave->bytesPerSample;

	for(int i = 0; i < wave->sampleRate * numChannels * bytesPerSample; i += 1024 * numChannels * bytesPerSample){//initializing buffer
		struct Node *temp = init();
		temp->sampleNumber = i;
		temp->energy = instantEnergy(wave, i);
		initAdd(tailBuffer, temp);
	}

	struct Node *headBeats = init();// creates list of beats
	struct Node *tailBeats = init();	

	headBeats->next = tailBeats;
	tailBeats->prev = headBeats;

	for(int i = wave->sampleRate; i < wave->dataSize; i += 1024 * numChannels * bytesPerSample){//traverses songs and finds possible beats
		struct Node *forBuffer = init();
		forBuffer->sampleNumber = i;
		forBuffer->energy = instantEnergy(wave, i);

		add(tailBuffer, headBuffer, forBuffer);

		double averageEnergy = average(headBuffer);
		double multiplier = retc(headBuffer);

		//use nathan and otto's multipler
		if(forBuffer->energy > multiplier*averageEnergy){//node is a beat
			struct Node *forBeats = copy(forBuffer);
			initAdd(tailBeats, forBeats);	//initAdd doesn't delete from list	
		}
	}
	
	freeUp(headBuffer);
	return headBeats;
}

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("%dYou need to put in an argument", argc);
		exit(1);
	}
	FILE *f = fopen(argv[1], "rb");

	WAVE *wave = readWave(f);

	struct Node *head = findBeats(wave);
	struct Node *test = head->next;
	while (test->sampleNumber!=-1){
	    printf("%ld\n", test->sampleNumber - test->prev->sampleNumber);
	    test=test->next;
	}

	fclose(f);
	free(wave->data);
	free(wave);
	freeUp(head);
}
