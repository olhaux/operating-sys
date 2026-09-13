#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int num_threads = 0;
long array_len = 0;
int num_bins = 0;
float *array = NULL;
long **local_hist = NULL;   // one histogram per thread indexed by id

int next_id = 0;
pthread_mutex_t id_lock = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(void *arg);

double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// clamp v == 1.0 into the last bin
int bin_of(float v)
{
    int b = (int)(v * (float)num_bins);
    if (b >= num_bins) b = num_bins - 1;
    if (b < 0) b = 0;
    return b;
}

void print_hist(const char *label, const long *h)
{
    printf("%s\n", label);
    for (int b = 0; b < num_bins; b++)
        printf("bin %d: %ld\n", b, h[b]);
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s <num_threads> <array_length> <num_bins>\n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);
    array_len = atol(argv[2]);
    num_bins = atoi(argv[3]);
    if (num_threads < 1 || array_len < 1 || num_bins < 1) {
        fprintf(stderr, "all three arguments must be >= 1\n");
        return 1;
    }
    if (num_threads > array_len) num_threads = (int)array_len;

    array = malloc(array_len * sizeof(float));
    if (!array) { perror("malloc"); return 1; }
    srand(12345); // fixed seed
    for (long i = 0; i < array_len; i++)
        array[i] = (float)rand() / (float)RAND_MAX;

    /* serial histogram */
    long *hist_serial = calloc(num_bins, sizeof(long));
    if (!hist_serial) { perror("calloc"); return 1; }
    double t0 = now_sec();
    for (long i = 0; i < array_len; i++)
        hist_serial[bin_of(array[i])]++;
    double time_serial = now_sec() - t0;
    print_hist("Serial histogram:", hist_serial);
    printf("Serial time = %.3f ms\n\n", time_serial * 1000.0);

    /* parallel histogram */
    pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
    long *hist_parallel = calloc(num_bins, sizeof(long));
    local_hist = malloc(num_threads * sizeof(long *));
    if (!workers || !hist_parallel || !local_hist) { perror("malloc"); return 1; }
    for (int i = 0; i < num_threads; i++) {
        local_hist[i] = calloc(num_bins, sizeof(long));
        if (!local_hist[i]) { perror("calloc"); return 1; }
    }

    t0 = now_sec(); // includes thread creation
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&workers[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(workers[i], NULL);

    // add up the per-thread histograms
    for (int i = 0; i < num_threads; i++)
        for (int b = 0; b < num_bins; b++)
            hist_parallel[b] += local_hist[i][b];
    double time_parallel = now_sec() - t0;

    print_hist("Parallel histogram:", hist_parallel);
    printf("Parallel time = %.3f ms\n", time_parallel * 1000.0);

    for (int i = 0; i < num_threads; i++)
        free(local_hist[i]);
    free(local_hist);
    free(array);
    free(hist_serial);
    free(hist_parallel);
    free(workers);
    return 0;
}

void *thread_func(void *arg)
{
    (void)arg; // not used since the id comes from next_id

    pthread_mutex_lock(&id_lock);
    int my_id = next_id;
    next_id++;
    pthread_mutex_unlock(&id_lock);

    // the first rem threads take one extra element
    long base = array_len / num_threads;
    long rem = array_len % num_threads;
    long start = my_id * base + (my_id < rem ? my_id : rem);
    long end = start + base + (my_id < rem ? 1 : 0);

    // count in a local array first so threads do not share cache lines
    long counts[num_bins];
    for (int b = 0; b < num_bins; b++)
        counts[b] = 0;
    for (long i = start; i < end; i++)
        counts[bin_of(array[i])]++;

    for (int b = 0; b < num_bins; b++)
        local_hist[my_id][b] = counts[b];

    pthread_exit(0);
}
