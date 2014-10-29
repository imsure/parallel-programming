/**
 * Matrix (N*N) multiplication with Open MP.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "omp.h"

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

int main( int argc, char *argv[] )
{
  double **matrix1, **matrix2, **matrix3;
  int size, i, j, k, chunksize, numthreads;
  double sum = 0;
  struct timeval tstart, tend;
  double exectime;

  if (argc != 3) {
    fprintf( stderr, "%s <matrix size> <number of thread>\n", argv[0] );
    return -1;
  }

  size = atoi( argv[1] );
  numthreads = atoi( argv[2] );

  if (size % numthreads != 0) {
    fprintf( stderr, "matrix size %d must be a multiple of number of threads %d!\n",
	     size, numthreads );
    return -1;
  }
  omp_set_num_threads( numthreads );
  chunksize = size / numthreads;

  matrix1 = allocate_matrix( size );
  matrix2 = allocate_matrix( size );
  matrix3 = allocate_matrix( size );
  
  init_matrix( matrix1, size );
  init_matrix( matrix2, size );

  if ( size <= 10 ) {
    printf( "Matrix 1:\n" );
    print_matrix( matrix1, size );
    printf( "Matrix 2:\n" );
    print_matrix( matrix2, size );
  }

  gettimeofday( &tstart, NULL );
  
#pragma omp parallel for shared(matrix1, matrix2, matrix3, chunksize) \
  private(i,j,k,sum) schedule(static, chunksize)
  for (i = 0; i < size; ++i) { // hold row index of 'matrix1'
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
  gettimeofday( &tend, NULL );
  
  if ( size <= 10 ) {
    printf( "Matrix 3:\n" );
    print_matrix( matrix3, size );
  }

  exectime = (tend.tv_sec - tstart.tv_sec) * 1000.0; // sec to ms
  exectime += (tend.tv_usec - tstart.tv_usec) / 1000.0; // us to ms   

  printf( "Number of MPI ranks: 0\tNumber of threads: %d\tExecution time:%.3lf sec\n",
          numthreads, exectime/1000.0);

  return 0;
}
