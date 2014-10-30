/**
 * MPI solution for red-black grid computation.
 *
 * Author: Shuo Yang
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int num_nodes;

double **init_grid( int gridsize, int strip_size,
		    int myrank, int num_nodes );

void print_grid( double **grid, int myrank,
		 int gridsize, int strip_size, int num_nodes );

void compute_grid_red( double **grid, int gridsize, int strip_size, int myrank );
void compute_grid_black( double **grid, int gridsize, int strip_size, int myrank );
void exchange_rows( double **grid, int gridsize, int strip_size, int rank );
double compute_grid_red_max( double **grid, int gridsize, int strip_size, int myrank );
double compute_grid_black_max( double **grid, int gridsize, int strip_size, int myrank, double maxdiff );

double MAX( double a, double b ) {
  return ( a > b )? a : b;
}

int main(int argc, char *argv[])
{
  int myrank, gridsize, num_iters, err_code, strip_size, iter;
  double **grid;
  double start_time, end_time;

  if (argc != 3) {
    printf( "Please pass the right arguments!\n" );
    printf( "Usage: mpirun -n <number of nodes> %s <gridsize> <number of iterations>\n", argv[0] );
    return -1;
  }

  gridsize = atoi( argv[1] );
  num_iters = atoi( argv[2] );

  MPI_Init( NULL, NULL );
  MPI_Comm_size( MPI_COMM_WORLD, &num_nodes );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

  if (myrank == 0 && gridsize % num_nodes != 0) {
    MPI_Abort(MPI_COMM_WORLD, err_code);
  }

  // start timer
  if (myrank == 0) {
    start_time = MPI_Wtime();
  }

  strip_size = gridsize / num_nodes;
  grid = init_grid( gridsize+2, strip_size+2, myrank, num_nodes );

  for (iter = 0; iter < num_iters; ++iter) {
    // compute red points
    compute_grid_red( grid, gridsize+2, strip_size+2, myrank );
    // send updates to neighbors
    exchange_rows( grid, gridsize+2, strip_size+2, myrank );
    // compute black points
    compute_grid_black( grid, gridsize+2, strip_size+2, myrank );
    // send updates to neighbors
    exchange_rows( grid, gridsize+2, strip_size+2, myrank );
  }

  double maxdiff, maxdiff_global;
  maxdiff = compute_grid_red_max( grid, gridsize+2, strip_size+2, myrank );
  exchange_rows( grid, gridsize+2, strip_size+2, myrank );
  maxdiff = compute_grid_black_max( grid, gridsize+2, strip_size+2, myrank, maxdiff );

  MPI_Reduce(&maxdiff, &maxdiff_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  //print_grid( grid, myrank, gridsize+2, strip_size+2, num_nodes );

  // stop timer
  if (myrank == 0) {
    end_time = MPI_Wtime();
    printf( "Number of MPI ranks: %d\tNumber of threads: 0\tExecution time:%.3lf sec\tMax difference:%lf\n",
	    num_nodes, end_time-start_time, maxdiff_global);
  }
  
  MPI_Finalize();
}

void compute_grid_red( double **grid, int gridsize, int strip_size, int myrank )
{
  int i, j, jstart;

  for (i = 1; i < strip_size-1; i++) {
    if (i % 2 == 1) jstart = 1; // odd row
    else jstart = 2; // even row
    
    for (j = jstart; j < gridsize-1; j += 2) {
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
    }
  }
}

double compute_grid_red_max( double **grid, int gridsize, int strip_size, int myrank )
{
  int i, j, jstart;
  double old, maxdiff = 0.0;

  for (i = 1; i < strip_size-1; i++) {
    if (i % 2 == 1) jstart = 1; // odd row
    else jstart = 2; // even row
    
    for (j = jstart; j < gridsize-1; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      maxdiff = MAX( maxdiff, fabs(old - grid[i][j]) );
    }
  }
  //printf( "rank %d maxdiff: %lf\n", myrank, maxdiff);
  return maxdiff;
}

void compute_grid_black( double **grid, int gridsize, int strip_size, int myrank )
{
  int i, j, jstart;

  for (i = 1; i < strip_size-1; i++) {
    if (i % 2 == 1) jstart = 2; // odd row
    else jstart = 1; // even row
    
    for (j = jstart; j < gridsize-1; j += 2) {
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
    }
  }
}

double compute_grid_black_max( double **grid, int gridsize, int strip_size, int myrank, double maxdiff )
{
  int i, j, jstart;
  double old;

  for (i = 1; i < strip_size-1; i++) {
    if (i % 2 == 1) jstart = 2; // odd row
    else jstart = 1; // even row
    
    for (j = jstart; j < gridsize-1; j += 2) {
      old = grid[ i ][ j ];
      grid[ i ][ j ] = ( grid[ i-1 ][ j ] + grid[ i+1 ][ j ] +
			 grid[ i ][ j-1 ] + grid[ i ][ j+1 ] ) * 0.25;
      maxdiff = MAX( maxdiff, fabs(old - grid[i][j]) );
    }
  }

  //printf( "rank %d maxdiff: %lf\n", myrank, maxdiff);
  return maxdiff;
}

/**
 * This is not working for message beyond certain size because:
 * Suppose num_nodes=2, we have two nodes rank 0 and rank 1.
 *         rank 0               rank 1
 *         MPI_Send(1)          MPI_Send(0)
 *         MPI_Recv(1)          MPI_Recv(0)
 *
 * This is a typical error occurs often in MPI programming due to poor
 * semantics of MPI_Send. Some implementation of MPI may utilitze an
 * internal buffer for sending messages. But if the message we are going
 * to send is bigger than the buffer, MPI_Send will be a blocking send,
 * waiting for a ACK from receiver. Otherwise MPI_Send is nonblocking and
 * returns immediately when the buffer (message buffer, not the internal buffer)
 * is available to use.
 *
 * This should be the reason why the code does not work with large input size!!!
 */
#if 0
void exchange_rows( double **grid, int gridsize, int strip_size, int rank )
{
  if (rank != 0)
    MPI_Send( grid[1], gridsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD );
  if (rank != num_nodes-1)
    MPI_Send( grid[strip_size-2], gridsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD );
  if (rank != 0)
    MPI_Recv( grid[0], gridsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
  if (rank != num_nodes-1)
    MPI_Recv( grid[strip_size-1], gridsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
}
#endif

void exchange_rows( double **grid, int gridsize, int strip_size, int rank )
{
  MPI_Request request_up;
  MPI_Request request_down;
  MPI_Status  status;

  if (rank != 0) {
    MPI_Isend( grid[1], gridsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &request_up );
  }
  if (rank != num_nodes-1) {
    MPI_Isend( grid[strip_size-2], gridsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &request_down );
  }
  if (rank != 0)
    MPI_Recv( grid[0], gridsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
  if (rank != num_nodes-1)
    MPI_Recv( grid[strip_size-1], gridsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

  if (rank != 0) {
    MPI_Wait( &request_up, &status );
  }
  if (rank != num_nodes-1) {
    MPI_Wait( &request_down, &status );
  }
}

void print_grid( double **grid, int myrank,
		 int gridsize, int strip_size, int num_nodes )
{
  int i,j;
  
  printf( "rank: %d\n", myrank );
  for (i = 0; i < strip_size; ++i) {
    printf( "row %d: ", i );
    for (j = 0; j < gridsize; ++j) {
      printf( "%lf ", grid[ i ][ j ] );
    }
    putchar('\n');
  }
}

double **init_grid( int gridsize, int strip_size,
		    int myrank, int num_nodes )
{
  int i, j;
  double ** outer_ptr;
  double *vals; // to allocate a contiguously array

  vals = (double *) malloc( gridsize * strip_size * sizeof(double) );
  outer_ptr = (double **) malloc( strip_size * sizeof(double*) );

  for (i = 0; i < strip_size; ++i) {
    outer_ptr[ i ] = &(vals[i * gridsize]);
  }

  for (i = 0; i < strip_size; ++i) {
    for (j = 0; j < gridsize; ++j) {
      if (j == 0 || j == (gridsize-1))
	outer_ptr[ i ][ j ] = 1.0;
      else
	outer_ptr[ i ][ j ] = 0.0;
    }
  }

  if (myrank == 0) {
    for (j = 0; j < gridsize; ++j) {
      outer_ptr[ 0 ][ j ] = 1.0;
    }
  }

  if (myrank == num_nodes-1) {
    for (j = 0; j < gridsize; ++j) {
      outer_ptr[ strip_size-1 ][ j ] = 1.0;
    }
  }

  return outer_ptr;
}
