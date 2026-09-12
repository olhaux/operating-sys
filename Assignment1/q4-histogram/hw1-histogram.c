/*
 * hw1-histogram.c  --  Q4: multi-threaded histogram of an array
 *
 * Usage: ./hw1-histogram <num_threads> <array_length> <num_bins>
 *
 * The master builds an array of random floats in [0,1] and histograms it
 * serially. Then num_threads workers each histogram a disjoint slice into a
 * PRIVATE bin array; the master reduces those into the final histogram.
 *
 * The private-then-reduce pattern is the whole point: a single shared
 * histogram would make hist[b]++ a data race, because ++ is a
 * read-modify-write, not an atomic operation.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static float *array = NULL;   /* READ-ONLY once filled by the master */
static long   array_len = 0;
static int    num_bins  = 0;
static int    num_threads = 0;

typedef struct {
    int   id;
    long  start;
    long  end;
    long *hist;   /* private bin array, calloc'd per thread */
} targ_t;

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Map a value in [0,1] to a bin index. A value of exactly 1.0 would compute
 * index == num_bins and write one past the end, so it is clamped. */
static inline int bin_of(float v)
{
    int b = (int)(v * (float)num_bins);
    if (b >= num_bins) b = num_bins - 1;
    if (b < 0)         b = 0;
    return b;
}

static void print_hist(const char *label, const long *h)
{
    printf("%s\n", label);
    for (int b = 0; b < num_bins; b++)
        printf("  bin %2d [%.4f, %.4f) : %ld\n",
               b, (double)b / num_bins, (double)(b + 1) / num_bins, h[b]);
}

void *thread_func(void *arg)
{
    targ_t *t = (targ_t *)arg;

    for (long i = t->start; i < t->end; i++)
        t->hist[bin_of(array[i])]++;   /* private bins -> no lock needed */

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s <num_threads> <array_length> <num_bins>\n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);
    array_len   = atol(argv[2]);
    num_bins    = atoi(argv[3]);
    if (num_threads < 1 || array_len < 1 || num_bins < 1) {
        fprintf(stderr, "all three arguments must be >= 1\n");
        return 1;
    }
    if (num_threads > array_len) num_threads = (int)array_len;

    printf("threads = %d, array_length = %ld, bins = %d\n\n",
           num_threads, array_len, num_bins);

    array = malloc((size_t)array_len * sizeof(float));
    if (!array) { perror("malloc array"); return 1; }
    srand(12345);
    for (long i = 0; i < array_len; i++)
        array[i] = (float)rand() / (float)RAND_MAX;

    /* ---- serial histogram ---- */
    long *hist_serial = calloc((size_t)num_bins, sizeof(long));
    if (!hist_serial) { perror("calloc"); return 1; }

    double t0 = now_sec();
    for (long i = 0; i < array_len; i++)
        hist_serial[bin_of(array[i])]++;
    double time_serial = now_sec() - t0;

    print_hist("Serial histogram:", hist_serial);
    printf("Serial   time = %.3f ms\n\n", time_serial * 1000.0);

    /* ---- parallel histogram ---- */
    pthread_t *workers = malloc((size_t)num_threads * sizeof(pthread_t));
    targ_t    *targs   = malloc((size_t)num_threads * sizeof(targ_t));
    long      *hist_parallel = calloc((size_t)num_bins, sizeof(long));
    if (!workers || !targs || !hist_parallel) { perror("malloc"); return 1; }

    long base = array_len / num_threads;
    long rem  = array_len % num_threads;
    long pos  = 0;

    t0 = now_sec();

    for (int i = 0; i < num_threads; i++) {
        long len = base + (i < rem ? 1 : 0);
        targs[i].id    = i;
        targs[i].start = pos;
        targs[i].end   = pos + len;
        targs[i].hist  = calloc((size_t)num_bins, sizeof(long));
        if (!targs[i].hist) { perror("calloc private hist"); return 1; }
        pos += len;

        if (pthread_create(&workers[i], NULL, thread_func, &targs[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
        for (int b = 0; b < num_bins; b++)      /* master reduces */
            hist_parallel[b] += targs[i].hist[b];
    }
    double time_parallel = now_sec() - t0;

    print_hist("Parallel histogram:", hist_parallel);
    printf("Parallel time = %.3f ms\n", time_parallel * 1000.0);
    printf("speedup = %.2fx\n", time_serial / time_parallel);

    int identical = (memcmp(hist_serial, hist_parallel,
                            (size_t)num_bins * sizeof(long)) == 0);
    printf("histograms identical: %s\n", identical ? "YES" : "NO");

    for (int i = 0; i < num_threads; i++) free(targs[i].hist);
    free(array); free(hist_serial); free(hist_parallel);
    free(workers); free(targs);
    return 0;
}
