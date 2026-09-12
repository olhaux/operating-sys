#include <unistd.h>

#define N 3

int main () {

  for (int i=0;i<N;i++)
  {
    fork();
    fork();
  }

  // keep the processes alive so they show up in top
  sleep(30);

  return 0;
}
