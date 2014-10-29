/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>

int main( int argc, char *argv[] )
{
  int numtasks, myrank, len;
  char hostname[ MPI_MAX_PROCESSOR_NAME ];

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task
  MPI_Get_processor_name( hostname, &len ); // get processor name running current MPI task

  if ( myrank == 0 )
    printf( "Number of task: %d\n", numtasks );

  printf( "Rank %d is running on %s\n", myrank, hostname );

  MPI_Finalize();
}
