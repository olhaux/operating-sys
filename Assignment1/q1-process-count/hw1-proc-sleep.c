#include <unistd.h>

#define N 3

int main () {

  for (int i=0;i<N;i++)
  {
    fork();
    fork();
  }

  /* Added for the second Q1.2 check. Keeps every process alive for 30 s so
   * the Tasks: line in top can show all of them at once. It runs after the
   * loop, so it does not change how many are created. */
  sleep(30);

  return 0;
}
