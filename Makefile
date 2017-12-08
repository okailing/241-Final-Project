all: alg

alg.o: alg.c waveparse.h
	gccx -c -lm alg.c

alg: alg.o waveparse.o
	gccx -o alg alg.o waveparse.o -lm

waveparse.o: waveparse.c waveparse.h
	gccx -c waveparse.c

