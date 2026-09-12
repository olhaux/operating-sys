/*
 * hw1-parallel-sum.c  --  Q3: multi-threaded sum of an array
 *
 * Master thread builds an array of ARRAY_LEN random floats in [0,1], sums it
 * serially, then splits the array across num_threads workers that each sum a
 * disjoint slice. The master aggregates the partial sums. Both phases timed.
 *
 * Build: gcc -O2 -Wall -pthread -o hw1-parallel-sum hw1-parallel-sum.c
 * Usage: ./hw1-parallel-sum <num_threads>
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_LEN 1000301   /* deliberately not divisible by 2 or 4 */

static float *array = NULL;   /* READ-ONLY once the master has filled it */
static int num_threads = 0;   /* READ-ONLY once parsed from argv          */

/* Per-thread argument block. Each worker touches only its own struct, so no
 * lock is needed: the threads share the struct *array* but never the same
 * element. */
typedef struct {
    int    id;        /* unique id in [0, num_threads-1] */
    long   start;     /* first index of this thread's slice (inclusive) */
    long   end;       /* last index of this thread's slice  (exclusive) */
    double partial;   /* written by this thread, read by master after join */
} targ_t;

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void *thread_func(void *arg)
{
    targ_t *t = (targ_t *)arg;
    double my_sum = 0.0;

    for (long i = t->start; i < t->end; i++)
        my_sum += array[i];

    t->partial = my_sum;
    printf("Thread %d sum = %f   (elements [%ld, %ld), %ld items)\n",
           t->id, my_sum, t->start, t->end, t->end - t->start);
    pthread_exit(0);
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

    /* ---- master builds the array ---- */
    array = malloc((size_t)ARRAY_LEN * sizeof(float));
    if (!array) { perror("malloc array"); return 1; }

    srand(12345);   /* fixed seed so runs are comparable to each other */
    for (long i = 0; i < ARRAY_LEN; i++)
        array[i] = (float)rand() / (float)RAND_MAX;

    /* ---- serial sum ---- */
    double t0 = now_sec();
    double sum_serial = 0.0;
    for (long i = 0; i < ARRAY_LEN; i++)
        sum_serial += array[i];
    double time_serial = now_sec() - t0;

    printf("Serial   Sum = %f, time = %.3f ms\n", sum_serial, time_serial * 1000.0);

    /* ---- parallel sum ---- */
    pthread_t *workers = malloc((size_t)num_threads * sizeof(pthread_t));
    targ_t    *targs   = malloc((size_t)num_threads * sizeof(targ_t));
    if (!workers || !targs) { perror("malloc workers"); free(array); return 1; }

    /* Split ARRAY_LEN into num_threads slices. ARRAY_LEN may not divide
     * evenly, so the first (ARRAY_LEN % num_threads) threads take one extra
     * element. This guarantees the slices tile the array exactly. */
    long base = ARRAY_LEN / num_threads;
    long rem  = ARRAY_LEN % num_threads;
    long pos  = 0;

    t0 = now_sec();   /* timer includes thread creation -- that is the honest cost */

    for (int i = 0; i < num_threads; i++) {
        long len = base + (i < rem ? 1 : 0);
        targs[i].id      = i;
        targs[i].start   = pos;
        targs[i].end     = pos + len;
        targs[i].partial = 0.0;
        pos += len;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (pthread_create(&workers[i], &attr, thread_func, &targs[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
        pthread_attr_destroy(&attr);
    }

    double sum_parallel = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
        sum_parallel += targs[i].partial;   /* master aggregates */
    }
    double time_parallel = now_sec() - t0;

    printf("Parallel Sum = %f, time = %.3f ms\n", sum_parallel, time_parallel * 1000.0);
    printf("difference (serial - parallel) = %.9f\n", sum_serial - sum_parallel);
    printf("speedup = %.2fx\n", time_serial / time_parallel);

    free(array);
    free(workers);
    free(targs);
    return 0;
}
