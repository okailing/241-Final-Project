all: alg

alg.o: alg.c waveparse.h
	gccx -c alg.c

alg: alg.o waveparse.o
	gccx -o alg alg.o waveparse.o

waveparse.o: waveparse.c waveparse.h
	gccx -c waveparse.c

clean:
	rm *.o alg
