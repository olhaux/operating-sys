#include <stdio.h>
#include <unistd.h>

#define N 3

int main () {

  for (int i=0;i<N;i++)
  {
    fork();
    fork();
  }

  // print each pid once, count with: ./hw1-proc-count | wc -l
  printf("%d\n", getpid());

  return 0;
}
