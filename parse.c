#include <stdio.h>
#include <stdlib.h>

#define READ(A) fread(buffer,(A),1,wavFile)

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("No file name specified");
		exit(1);
	}

	FILE *wavFile = fopen(argv[1], "rb");
	char *buffer = malloc(1024);
	READ(4);
	printf("ChunkID: %s\n", buffer);	
	READ(4);
	printf("Chunk Size: %d\n", *(int *)buffer);
	READ(4);
	printf("Format: %s\n", buffer);
	READ(4);
	printf("Subchunk1ID: %s\n", buffer);
	READ(4);
	printf("Subchunk1Size: %d\n", *(int *) buffer);
	READ(2);
	printf("AudioFormat: %hd\n", *(short *) buffer);
	short numChannels;
	fread(&numChannels, 2, 1, wavFile);
	printf("NumChannels: %hd\n", numChannels);
	READ(4);READ(4); //skipping SampleRate
	printf("ByteRate: %d\n", *(int *) buffer);
	READ(2);//skipping block allign
	short bitsPerSample;
	fread(&bitsPerSample, 2, 1, wavFile);
	short bytesPerSample = bitsPerSample / 8;
	printf("BitsPerSample: %hd\n", bitsPerSample);
	READ(4);
	printf("Subchunk2ID: %s\n", buffer);
	int dataSize;
	fread(&dataSize, 4, 1, wavFile);
	
	char data_buffer[bytesPerSample];
	for(int i = 0; i < dataSize; i +=bytesPerSample*numChannels){
		printf("Sample %d:\n", i/(bytesPerSample*numChannels));
		for(int j = 0; j < numChannels; j++){
			fread(buffer, bytesPerSample, 1, wavFile);
			int data = buffer[0] | (buffer[1]<<8);
			printf("Channel %d: %d\t", j, data);
		}
		putchar('\n');
		getchar();
	}
}
