#include "waveparse.h"

#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
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

	//free(toDelete);//remove new from tail
	//freeing up everything is gonna be a mess
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
		struct Node *newNode = init();
		newNode->sampleNumber = i;
		newNode->energy = instantEnergy(wave, i);

		add(tailBuffer, headBuffer, newNode);

		double averageEnergy = average(headBuffer);
		double multiplier = retc(headBuffer);

		//use nathan and otto's multipler
		if(newNode->energy > multiplier*averageEnergy){//node is a beat
			initAdd(tailBeats, newNode);	//initAdd doesn't delete from list	
		}
	}

	return headBeats;
}

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("%dYou need to put in an argument", argc);
		exit(1);
	}
	FILE *f = fopen(argv[1], "rb");

	WAVE *wave = readWave(f);

	struct Node *head = init();
	struct Node *tail = init();
	head->next = tail;
	tail->prev = head;

	int i;
	for(i=0;i<wave->sampleRate-1024;i=i+1024){//I subtract 1024 from sampleRate to account for the fact that 1024 doesn't envenly divide the sampleRate
		struct Node *tmp = init();
	    tmp->sampleNumber = i;
	    tmp->energy=instantEnergy(wave,i);
		initAdd(tail, tmp);    
	}

	struct Node *test = findBeats(wave);
	test = test->next;
	while (test->sampleNumber!=-1){
		break;
	    printf("%ld\n", test->sampleNumber);
	    test=test->next;
	}

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
