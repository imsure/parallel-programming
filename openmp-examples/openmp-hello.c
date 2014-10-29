#include <omp.h>
#include <stdio.h>

int main ()
{
  int nthreads, tid;

  omp_set_num_threads( 4 );
  /* Fork a team of threads with each thread having a private tid variable */
#pragma omp parallel private(tid)
  {

    /* Obtain and print thread id */
    tid = omp_get_thread_num();
    printf("Hello World from thread = %d\n", tid);

    /* Only master thread does this */
    if (tid == 0) 
      {
	nthreads = omp_get_num_threads();
	printf("Number of threads = %d\n", nthreads);
      }

  }  /* All threads join master thread and terminate */

}
