/**
 * Test the thread support level of the MPI library.
 */

#include "mpi.h"
#include <stdio.h>
 
int main( int argc, char *argv[] )
{
  int provided, claimed;
 
  /*** Select one of the following
       MPI_Init_thread( 0, 0, MPI_THREAD_SINGLE, &provided );
       MPI_Init_thread( 0, 0, MPI_THREAD_FUNNELED, &provided );
       MPI_Init_thread( 0, 0, MPI_THREAD_SERIALIZED, &provided );
       MPI_Init_thread( 0, 0, MPI_THREAD_MULTIPLE, &provided );
  ***/ 

  MPI_Init_thread(0, 0, MPI_THREAD_MULTIPLE, &provided );
  MPI_Query_thread( &claimed );
  printf( "Query thread level= %d  Init_thread level= %d\n", claimed, provided );
 
  MPI_Finalize();
}

