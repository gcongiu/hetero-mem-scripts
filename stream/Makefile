.PHONY=stream_omp

CC=icc

all: stream_omp

stream_omp: stream_omp.c
	$(CC) $^ -o $@ -g -O3 -fopenmp

clean:
	rm -f stream_omp
