/**
 * Test of functionality of MPI_Alltoall
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char *argv[] )
{
  char *send_buf, *recv_buf;
  int myrank, num_nodes, i, len, chunk;
  MPI_Status status;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &num_nodes );

  if ( myrank == 0 ) {
    if ( num_nodes != 3 ) {
      fprintf( stderr, "This test requires three nodes!\n" );
      MPI_Abort( MPI_COMM_WORLD, -1 );
      exit( -1 );
    }
  }

  chunk = 2;
  len = num_nodes;
  send_buf = (char *) malloc( (chunk*len+1) * sizeof(char) );
  recv_buf = (char *) malloc( (chunk*len+1) * sizeof(char) );
  if ( myrank == 0 )
    strcpy( send_buf, "abcdef" );
  else if ( myrank == 1 )
    strcpy( send_buf, "ABCDEF" );
  else if ( myrank == 2 )
    strcpy( send_buf, "#@%$&*" );
  
  printf( "Rank %d: before MPI_Alltoall, send buffer is: %s\n", myrank, send_buf );

  MPI_Barrier( MPI_COMM_WORLD );

  MPI_Alltoall( send_buf, chunk, MPI_CHAR, recv_buf, chunk, MPI_CHAR, MPI_COMM_WORLD );
  recv_buf[ chunk*len ] = 0;
  printf( "Rank %d: After MPI_Alltoall, receive buffer is: %s\n", myrank, recv_buf );

  MPI_Finalize();
}
