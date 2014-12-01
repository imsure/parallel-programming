/**
 * Test of functionality of MPI_Scatter
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
  int *send_buf, recv = 0;
  int myrank, num_nodes, i;
  MPI_Status status;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &num_nodes );

  if (myrank == 0) {
    send_buf = (int *) malloc( num_nodes * sizeof(int) );
    for (i = 0; i < num_nodes; ++i) {
      send_buf[ i ] = i + 1;
    }
  }

  printf( "Rank %d: Before scattering, receive buffer is: %d\n", myrank, recv );
  MPI_Barrier( MPI_COMM_WORLD );

  /* Rank 0 scatter a int to all the ranks. Each rank gets
     a value that equals its rank + 1 */
  MPI_Scatter( send_buf, 1, MPI_INT, &recv, 1,
	       MPI_INT, 0, MPI_COMM_WORLD );

  MPI_Barrier( MPI_COMM_WORLD );
  printf( "Rank %d: After scattering, receive buffer is: %d\n", myrank, recv );

  MPI_Finalize();
}
