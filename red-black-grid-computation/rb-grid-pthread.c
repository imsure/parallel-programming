/**
 * Red/Black grid computation using Pthread for the shared memory
 * parallel program.
 *
 * Author: Shuo Yang
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

int num_iters; // number of iterations
int gridsize; // the size of the grid
double ** grid; // shared grid
int num_threads; // number of threads
int height; // height of the grid for each thread to process
// maximum difference between old and new values among all the grid cells.
double * max_diff; 
volatile int * arrive; // arrive array for the dissemination barrier

double MAX( double a, double b ) {
  return ( a > b )? a : b;
}

/*
 * Sychronization barrier for thread 'i'.
 */
void dissem_barrier( int i )
{
  int j, look_at;
  for (j = 1; j <= ceil( log(num_threads)/log(2) ); ++j) {
    while (arrive[ i ] != 0) ;
    arrive[ i ] = j;
    look_at = ( i + (int)powl(2, j-1) ) % num_threads;
    while (arrive[ look_at ] != j) ;
    arrive[ look_at ] = 0;
  }
}

/**
 * Allocate a n*n grid
 */
double ** allocate_grid( int n )
{
  int i;
  double ** outer_ptr;
  double *vals; // to allocate a contiguously array

  vals = (double *) malloc( n * n * sizeof(double) );
  outer_ptr = (double **) malloc( n * sizeof(double*) );

  for (i = 0; i < n; ++i) {
    outer_ptr[ i ] = &(vals[i * n]);
  }

  return outer_ptr;
}

/*
 * Initialize grid given the index of the first row and last row.
 */
void init_grid( int first_row, int last_row )
{
  int i,j;
  /* Initialize grid, including boundaries. */
  for (i = first_row; i <= last_row; ++i ) {
    for (j = 0; j <= (gridsize+1); ++j) {
      if (i == 0 || i == (gridsize+1) ||
	  j == 0 || j == (gridsize+1) ) {
	grid[i][j] = 1;
      } else {
	grid[i][j] = 0;
      }
    }
  }
}

/*
 * Compute grid for one iteration, given the index of the first row and
 * last row, along with thread id 'id'.
 */
void grid_computation( int first_row, int last_row, int id )
{
  int jstart, i, j;
  
  /* Compute new values for red points in the grid strip.
     Note that red points only depend on black points. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 1; // odd row
    else jstart = 2; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
    }
  }

  /* Insert a barrier. Before computing the value for black points,
     we must make sure that all the red points have been computed because
     the value of black points depend on red points. */
  dissem_barrier( id );
    
  /* Compute new values for black points in the grid strip.
     Note that black points only depend on red points. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 2; // odd row
    else jstart = 1; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
    }
  }

  /* Insert a barrier. Before computing the value for red points,
     we must make sure that all the black points have been computed because
     the value of red points depend on black points. */
  dissem_barrier( id );
}
/**
 * Thread routine.
 * 'arg' is the index ranging from 0 to num_thread-1 to identify the thread.
 */
void * worker( void *arg )
{
  int id = *((int *) arg);
  int first_row = id * height + 1;
  int last_row = first_row + height - 1;
  int jstart, iter, i, j;
  double mydiff = 0.0, old;

  if (first_row == 1)
    init_grid( first_row-1, last_row );
  else if (last_row == gridsize)
    init_grid( first_row-1, last_row+1 );
  else
    init_grid( first_row, last_row );

  /* Insert a barrier to wait for all the other threads to finish the grid initialization. */
  dissem_barrier( id );

  for (iter = 1; iter <= num_iters; ++iter) {
    grid_computation( first_row, last_row, id );
  }

  /**
   * Do the iteration one more time to compute the max difference among each cell.
   */

  /* Compute new values for red points in the grid strip. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 1; // odd row
    else jstart = 2; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      mydiff = MAX( mydiff, fabs(old - grid[i][j]) );
    }
  }

  /* Insert a barrier. Before computing the value for black points,
     we must make sure that all the red points have been computed because
     the value of black points depend on red points. */
  dissem_barrier( id );
  
  /* Compute new values for black points in the grid strip. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 2; // odd row
    else jstart = 1; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      mydiff = MAX( mydiff, fabs(old - grid[i][j]) );
    }
  }

  max_diff[ id ] = mydiff;
  return NULL;
}

void print_grid( double **grid, int size )
{
  int i, j;
  for (i = 0; i < size; ++i) {
    for (j = 0; j < size; ++j) {
      printf( "%lf ", grid[i][j] );
    }
    putchar('\n');
  }
}

int main(int argc, char *argv[])
{
  int i;
  int *arg; // argument passed to thread
  double maxdiff = 0.0;
  struct timeval t_start, t_end; // for measuring execution time.
  double exec_time;

  /**
   * Parse the arguments.
   */
  if (argc != 4) {
    printf( "Please pass the right arguments!\n" );
    printf( "Usage: ./a.out <gridsize> <number of iterations> <number of cores>\n" );
    return -1;
  }

  gridsize = atoi( argv[1] );
  if (gridsize % 8 != 0) {
    fprintf( stderr, "grid size %d must be a multiple of 8!\n", gridsize);
    return -1;
  }
  num_iters = atoi( argv[2] );
  num_threads = atoi( argv[3] );

  height = gridsize / num_threads; 
  grid = allocate_grid( gridsize+2 ); // allocate (gridsize+2) x (gridsize+2) grid
  max_diff = (double *) malloc( num_threads * sizeof(double) );
  arrive = (int *) malloc ( num_threads * sizeof(int) );

  for (i = 0; i < num_threads; ++i) {
    arrive[ i ] = 0;
  }

  pthread_t threads[num_threads];
  gettimeofday( &t_start, NULL );
  for (i = 0; i < num_threads; ++i) {
    arg = (int *) malloc( sizeof(int) );
    *arg = i;
    pthread_create( &threads[i], NULL, worker, (void *)arg );
  }

  for (i = 0; i < num_threads; ++i) {
    pthread_join( threads[i], NULL );
  }
  gettimeofday( &t_end, NULL );

  exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0; // sec to ms
  exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms

  for (i = 0; i < num_threads; ++i) {
    maxdiff = MAX( maxdiff, max_diff[i] );
  }

  printf( "Number of MPI ranks: 0\tNumber of threads: %d\tExecution time:%.3lf sec\tMax difference:%lf\n",
	  num_threads, exec_time/1000.0, maxdiff);

  return 0;
}
