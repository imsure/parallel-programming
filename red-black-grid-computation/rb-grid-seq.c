#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int num_iters; // number of iterations
int gridsize; // the size of the grid

double MAX( double a, double b ) {
  return ( a > b )? a : b;
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

void init_grid( double **grid, int size )
{
  int i, j;
  for (i = 0; i < size; ++i) {
    for (j = 0; j < size; ++j) {
      if (i == 0 || i == (size-1) ||
	  j == 0 || j == (size-1) ) {
	grid[i][j] = 1;
      } else {
	grid[i][j] = 0;
      }
    }
  }
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
  double ** grid;
  double max_diff = 0.0, old;
  int first_row, last_row;
  int iter, jstart, i, j;
  struct timeval t_start, t_end; // for measuring execution time.
  double exec_time;

  /**
   * Parse the arguments.
   */
  if (argc != 3) {
    printf( "Please pass the right arguments!\n" );
    printf( "Usage: ./a.out <gridsize> <number of iterations>\n" );
    return -1;
  }

  gridsize = atoi( argv[1] );
  num_iters = atoi( argv[2] );

  grid = allocate_grid( gridsize+2 );
  gettimeofday( &t_start, NULL );
  init_grid( grid, gridsize+2 );

  first_row = 1;
  last_row = gridsize;

  for (iter = 1; iter <= num_iters; ++iter) {
    /* Compute new values for red points in the grid. */
    for (i = first_row; i <= last_row; ++i) {
      if (i % 2 == 1) jstart = 1; // odd row
      else jstart = 2; // even row

      for (j = jstart; j <= gridsize; j += 2) {
	grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			   grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      }
    }

    /* Compute new values for black points in the grid. */
    for (i = first_row; i <= last_row; ++i) {
      if (i % 2 == 1) jstart = 2; // odd row
      else jstart = 1; // even row

      for (j = jstart; j <= gridsize; j += 2) {
	grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			   grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      }
    }
  }

  /**
   * Do the iteration one more time to compute the max difference among each cell.
   */

  /* Compute new values for red points in the grid. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 1; // odd row
    else jstart = 2; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      max_diff = MAX( max_diff, fabs(old - grid[i][j]) );
    }
  }

  /* Compute new values for black points in the grid. */
  for (i = first_row; i <= last_row; ++i) {
    if (i % 2 == 1) jstart = 2; // odd row
    else jstart = 1; // even row

    for (j = jstart; j <= gridsize; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      max_diff = MAX( max_diff, fabs(old - grid[i][j]) );
    }
  }

  gettimeofday( &t_end, NULL );
  exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0; // sec to ms
  exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
  
  printf( "Number of MPI ranks: 0\tNumber of threads: 1\tExecution time:%.3lf sec\tMax difference:%lf\n",
	  exec_time/1000.0, max_diff);

  return 0;
}
