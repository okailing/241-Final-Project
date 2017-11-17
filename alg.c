#include "waveparse.h"

#include <inttypes.h>
#include <stdlib.h>

//calculates the "instant energy" at a given sample location
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

int main(int argc, char *argv[]){
	FILE *f = fopen(argv[1], "rb");

	WAVE *wave = readWave(f);

	printf("%ld\n", instantEnergy(wave, 0));
}
