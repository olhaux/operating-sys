#include <stdio.h>
#include <unistd.h>

#define N 3

int main () {

  for (int i=0;i<N;i++)
  {
    fork();
    fork();
  }

  /* Added for Q1.2. Every process prints its PID exactly once, after all the
   * forks, so counting the lines counts the processes:
   *     ./hw1-proc-count | wc -l
   * It runs after the loop, so it does not change how many are created. */
  printf("%d\n", getpid());

  return 0;
}
