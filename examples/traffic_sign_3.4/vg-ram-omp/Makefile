RELEASE = -O3 -ffast-math
#DEBUG = -g

build:
	gcc $(DEBUG) $(RELEASE) -fopenmp -o vgram-omp vgram_main_omp.c vgram_files.c -lm

clean:
	rm -f *.o vgram-omp
