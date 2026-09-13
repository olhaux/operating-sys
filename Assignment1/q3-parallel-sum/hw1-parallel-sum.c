#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_LEN 1000301

int num_threads = 0;
float *array = NULL;
double *partial = NULL;   // one partial sum per thread, indexed by id

int next_id = 0;
pthread_mutex_t id_lock = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(void *arg); /* the thread function */

double now_sec(void)
{
     struct timespec ts;
     clock_gettime(CLOCK_MONOTONIC, &ts);
     return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
     if (argc < 2) {
          fprintf(stderr, "usage: %s <num_threads>\n", argv[0]);
          return 1;
     }
     num_threads = atoi(argv[1]);
     if (num_threads < 1) {
          fprintf(stderr, "num_threads must be >= 1\n");
          return 1;
     }

     /* Initialize an array of random values */
     array = malloc(ARRAY_LEN * sizeof(float));
     if (!array) {
          perror("malloc");
          return 1;
     }
     srand(12345); // fixed seed
     for (long i = 0; i < ARRAY_LEN; i++)
          array[i] = (float)rand() / (float)RAND_MAX;

     /* Perform Serial Sum */
     double sum_serial = 0.0;
     double time_serial = 0.0;
     //Timer Begin
     double t0 = now_sec();
     for (long i = 0; i < ARRAY_LEN; i++)
          sum_serial += array[i];
     time_serial = now_sec() - t0;
     //Timer End
     printf("Serial Sum = %f, time = %.3f ms\n", sum_serial, time_serial * 1000.0);

     /* Create a pool of num_threads workers and keep them in workers */
     pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
     double time_parallel = 0.0;
     double sum_parallel = 0.0;
     partial = malloc(num_threads * sizeof(double));
     if (!workers || !partial) {
          perror("malloc");
          return 1;
     }

     //Timer Begin
     t0 = now_sec(); // includes thread creation
     for (int i = 0; i < num_threads; i++) {
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          if (pthread_create(&workers[i], &attr, thread_func, NULL) != 0) {
               perror("pthread_create");
               return 1;
          }
          pthread_attr_destroy(&attr);
     }

     for (int i = 0; i < num_threads; i++)
          pthread_join(workers[i], NULL);

     for (int i = 0; i < num_threads; i++)
          sum_parallel += partial[i];
     time_parallel = now_sec() - t0;
     //Timer End
     printf("Parallel Sum = %f, time = %.3f ms\n", sum_parallel, time_parallel * 1000.0);

     /*free up resources properly */
     free(array);
     free(partial);
     free(workers);
     return 0;
}

void *thread_func(void *arg) {
     (void)arg; // not used, the id comes from next_id

     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     pthread_mutex_lock(&id_lock);
     int my_id = next_id;
     next_id++;
     pthread_mutex_unlock(&id_lock);

     // the first rem threads take one extra element
     long base = ARRAY_LEN / num_threads;
     long rem = ARRAY_LEN % num_threads;
     long start = my_id * base + (my_id < rem ? my_id : rem);
     long end = start + base + (my_id < rem ? 1 : 0);

     /* Perform Partial Parallel Sum Here */
     double my_sum = 0.0;
     for (long i = start; i < end; i++)
          my_sum += array[i];

     partial[my_id] = my_sum;
     printf("Thread %d sum = %f\n", my_id, my_sum);
     pthread_exit(0);
}
