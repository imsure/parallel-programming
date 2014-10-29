seq-rb: rb-grid-seq.c
	gcc -O2 -o seq-rb rb-grid-seq.c

mt-rb: rb-grid-pthread.c
	gcc -O2 -o mt-rb rb-grid-pthread.c -lpthread -lm

dist-rb: rb-grid-mpi.c
	mpicc -O2 -o dist-rb rb-grid-mpi.c

hybrid-rb:
	mpicc -O2 -fopenmp -o hybrid-rb rb-grid-hybrid.c

clean:
	rm seq-rb mt-rb dist-rb hybrid-rb