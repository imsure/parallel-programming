/**
 * Matrix (N*N) multiplication with a single thread.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"
#include "omp.h"

#define TAG 10
#define DEBUG 0

double ** allocate_matrix( int size )
{
  /* Allocate 'size' * 'size' doubles contiguously. */
  double * vals = (double *) malloc( size * size * sizeof(double) );

  /* Allocate array of double* with size 'size' */
  double ** ptrs = (double **) malloc( size * sizeof(double*) );

  int i;
  for (i = 0; i < size; ++i) {
    ptrs[ i ] = &vals[ i * size ];
  }

  return ptrs;
}

void init_matrix( double **matrix, int size )
{
  int i, j;

  for (i = 0; i < size; ++i) {
    for (j = 0; j < size; ++j) {
      matrix[ i ][ j ] = 1.0;
    }
  }
}

void print_matrix( double **matrix, int size )
{
  int i, j;

  for (i = 0; i < size; ++i) {
    for (j = 0; j < size-1; ++j) {
      printf( "%lf, ", matrix[ i ][ j ] );
    }
    printf( "%lf", matrix[ i ][ j ] );
    putchar( '\n' );
  }
}

/**
 * Calculate:
 * matrix3 <- matrix1 * matrix2
 */
int main( int argc, char *argv[] )
{
  double **matrix1, **matrix2, **matrix3, *tmp;
  int size, i, j, k, myrank, numtasks, stripsize, chunksize, numthreads;
  double sum = 0, start_time, end_time;

  if (argc != 3) {
    fprintf( stderr, "%s <matrix size> <numthreads>\n", argv[0] );
    return -1;
  }

  size = atoi( argv[1] );
  numthreads = atoi( argv[2] );
  omp_set_num_threads( numthreads );

  MPI_Init( &argc, &argv );
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

  if ( myrank == 0 && size % numtasks != 0 ) {
    fprintf( stderr, "size %d must be a multiple of number of tasks %d\n", size, numtasks );
    MPI_Abort( MPI_COMM_WORLD, -1 );
  }

  stripsize = size / numtasks; // the size of the strip each rank works on.

  if ( myrank == 0 ) { // rank 0 allocate the entire matrix1 and matrix3
    matrix1 = allocate_matrix( size );
    matrix3 = allocate_matrix( size );
  
    init_matrix( matrix1, size ); // rank 0 initialize matrix 1
  } else {
    /* Allocate strip of matrix 1 other ranks need. */
    tmp = (double *) malloc( size * stripsize * sizeof(double) );
    matrix1 = (double **) malloc( stripsize * sizeof(double *) );
    for (i = 0; i < stripsize; ++i) {
      matrix1[ i ] = &( tmp[i * size] );
    }

    /* Allocate strip of matrix 3 other ranks need. */
    tmp = (double *) malloc( size * stripsize * sizeof(double) );
    matrix3 = (double **) malloc( stripsize * sizeof(double *) );
    for (i = 0; i < stripsize; ++i) {
      matrix3[ i ] = &( tmp[i * size] );
    }
  }

  /* Every rank allocates the entire 'matrix2' for calculation. */
  matrix2 = allocate_matrix( size );
  if (myrank == 0) { // only rank 0 initialize 'matrix2'.
    init_matrix( matrix2, size );
  }

  if (myrank == 0) {
    start_time = MPI_Wtime();
  }

  if (myrank == 0) {
    /* rank 0 dispatch values of strip of matrix1 to other ranks. */
    for ( i = 1; i < numtasks; ++i ) {
      MPI_Send( matrix1[i*stripsize], stripsize * size, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD );
#if DEBUG
      printf( "Sending to rank %d done!\n", i );
#endif
    }
  } else {
    // recevie strip from rank 0.
    MPI_Recv( matrix1[0], stripsize * size, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
#if DEBUG
    printf( "rank %d received strip from rank 0 done!\n", myrank );
#endif
  }

  // Broadcast values of 'matrix2' from rank 0 to all other ranks.
  MPI_Bcast( matrix2[0], size*size, MPI_DOUBLE, 0, MPI_COMM_WORLD );

  if ( myrank == 0 && size <= 10 ) {
      printf( "Matrix 1:\n" );
      print_matrix( matrix1, size );
      printf( "Matrix 2:\n" );
      print_matrix( matrix2, size );
    }

  chunksize = 20;
#pragma parallel for shared(matrix1, matrix2, matrix3, chunksize) \
  private(i, j, k, sum) schedule(static, chunksize)
  for (i = 0; i < stripsize; ++i) { // hold row index of 'matrix1'
    for (j = 0; j < size; ++j) { // hold column index of 'matrix2'
      sum = 0; // hold value of a cell
      /* one pass to sum the multiplications of corresponding cells
	 in the row vector and column vector. */
      for (k = 0; k < size; ++k) { 
	sum += matrix1[ i ][ k ] * matrix2[ k ][ j ];
      }
      matrix3[ i ][ j ] = sum;
    }
  }

  if ( myrank != 0 ) {
    // send strip of matrix3 to rank 0
    MPI_Send( matrix3[0], stripsize * size, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD );
#if DEBUG
    printf( "rank %d has sent strip of matrix3 to rank 0. Done!\n", myrank );
#endif
  }

  if ( myrank == 0 ) {
    for (i = 1; i < numtasks; ++i) {
      MPI_Recv( matrix3[i*stripsize], stripsize * size, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD,  MPI_STATUS_IGNORE );
#if DEBUG
      printf( "rank 0 received strip of matrix3 from rank %d done!\n", i );
#endif
    }
  }

  if ( myrank ==0 && size <= 10 ) {
    printf( "Matrix 3:\n" );
    print_matrix( matrix3, size );
  }

  if ( myrank == 0 ) {
    end_time = MPI_Wtime();
    printf( "Number of MPI ranks: %d\tNumber of threads: 0\tExecution time: %lf sec\n",
	    numtasks, end_time-start_time);
  }

  MPI_Finalize();
}
