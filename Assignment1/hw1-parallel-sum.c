#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
     
int num_threads = 0;
     
void *thread_func(); /* the thread function */

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);


     /* Initialize an array of random values */ 


     /* Perform Serial Sum */
     float  sum_serial = 0.0;
     double time_serial = 0.0;
     //Timer Begin

     //Timer End
     printf("Serial Sum = , time = %.3f \n", sum_serial, time_serial);

     /* Create a pool of num_threads workers and keep them in workers */ 
     pthread_t *workers;    
     double time_parallel = 0.0;
     double sum_parallel = 0.0;

     //Timer Begin

     for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, NULL); 
     }
     
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);


     //Timer End
     printf("Parallel Sum = , time = %.3f \n", sum_parallel, time_parallel);
     
     /*free up resources properly */
     
}

void *thread_func() { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     int my_id;


     /* Perform Partial Parallel Sum Here */
     float my_sum = 0.0;


     printf("Thread %d sum = %f\n", my_id, my_sum);
     pthread_exit(0);
}