//Sam Barr

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define READ(A) fread(buffer,(A),1,waveFile)

typedef struct wave {
	int sampleRate;
	int dataSize;//or numSamples??
	short numChannels;
	short bytesPerSample;	

	char *data;
//	how to traverse data:

//	for(int i = 0; i < dataSize; i +=bytesPerSample*numChannels){
//		printf("Sample %d:\n", i/(bytesPerSample*numChannels));
//		char *sample = data + i;
//		for(int j = 0; j < numChannels; j++){
//			char *channelData = sample + j;
//			printf("Channel %d: %d\t", j, *channelData);
//		}
//	}

} WAVE;

WAVE *readWave(FILE * waveFile){
	char *buffer = malloc(5*sizeof(char));
	*(buffer + 4) = '\0';

	READ(4);//ChunkID
	assert(strcmp(buffer, "RIFF")==0);

	READ(4);//Chunk Size

	READ(4);//Format
	assert(strcmp(buffer, "WAVE")==0);

	READ(4);//SubChunk1ID

	READ(4);//SubChunk1Size

	READ(2);//AudioFormat (should be 1)
	assert(*(short *)buffer == 1);

	short numChannels;
	fread(&numChannels, 2, 1, waveFile);

	int sampleRate;
	fread(&sampleRate, 4, 1, waveFile);

	READ(4);//ByteRate

	READ(2);//block allign

	short bitsPerSample;
	fread(&bitsPerSample, 2, 1, waveFile);
	short bytesPerSample = bitsPerSample / 8;

	READ(4);//SubChunk2ID

	int dataSize;
	fread(&dataSize, 4, 1, waveFile);

	char *data = malloc(dataSize);

	fread(data, dataSize, 1, waveFile);

	WAVE *wave = malloc(sizeof(WAVE));
	wave->sampleRate = sampleRate;
	wave->dataSize = dataSize;
	wave->numChannels = numChannels;
	wave->bytesPerSample = bytesPerSample;

	wave->data = data;

	free(buffer);
	return wave;
}

int main(){
	FILE *file = fopen("piano2.wav", "rb");
	WAVE *waveFile = readWave(file);
	char *data = waveFile->data;
	int dataSize = waveFile->dataSize;
	short bytesPerSample = waveFile->bytesPerSample;
	short numChannels = waveFile->numChannels;

	for(int i = 0; i < dataSize; i += bytesPerSample*numChannels){
		printf("Sample %d:\n", i/(bytesPerSample*numChannels));
		char *sample = data + i;
		for(int j = 0; j < numChannels*bytesPerSample; j += bytesPerSample){
			short *channelData = (short *)(sample + j);
			printf("Channel %d: %hd\t", j, *channelData);
		}
		getchar();
	}

	fclose(file);
	free(data);
	free(waveFile);
}
