#include <stdio.h>

typedef struct {
	int sampleRate;
	int dataSize;
	short numChannels;
	short bytesPerSample;

	char *data;
} WAVE;

WAVE *readWave(FILE *);
