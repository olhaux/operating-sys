// round robin version for 6.3
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 8
#define QUANTUM     1

typedef struct {
    int id, priority, burst_time, remaining;
    int completion_time, waiting_time, turnaround_time;
} thread_info_t;

thread_info_t threads[NUM_THREADS];

pthread_mutex_t sched_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  sched_cond = PTHREAD_COND_INITIALIZER;
int current_running_tid = -1;
int sim_clock = 0;
int last_run  = -1;

// start after whoever ran last
int pick_next() {
    for (int k = 1; k <= NUM_THREADS; k++) {
        int i = (last_run + k) % NUM_THREADS;
        if (threads[i].remaining > 0) {
            last_run = i;
            return i;
        }
    }
    return -1;
}

void schedule() {
    pthread_mutex_lock(&sched_lock);
    current_running_tid = pick_next();
    pthread_cond_broadcast(&sched_cond);
    pthread_mutex_unlock(&sched_lock);
}

void *thread_func(void *arg) {
    thread_info_t *t = (thread_info_t *)arg;

    while (1) {
        pthread_mutex_lock(&sched_lock);
        while (current_running_tid != t->id && t->remaining > 0)
            pthread_cond_wait(&sched_cond, &sched_lock);

        if (t->remaining <= 0) {
            pthread_mutex_unlock(&sched_lock);
            break;
        }

        // run at most one quantum
        int slice = (t->remaining < QUANTUM) ? t->remaining : QUANTUM;
        printf("Thread %d (prio %d) runs for %d units\n",
               t->id, t->priority, slice);

        sim_clock   += slice;
        t->remaining -= slice;

        if (t->remaining == 0) {
            t->completion_time = sim_clock;
            t->turnaround_time = t->completion_time;
            t->waiting_time    = t->turnaround_time - t->burst_time;
        }

        pthread_mutex_unlock(&sched_lock);
        schedule();
    }
    return NULL;
}

int main() {
    pthread_t tids[NUM_THREADS];
    int prios[]  = {3, 3, 2, 2, 2, 6, 7, 1};
    int bursts[] = {7, 6, 2, 8, 7, 1, 1, 9};

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].id = i;
        threads[i].priority = prios[i];
        threads[i].burst_time = threads[i].remaining = bursts[i];
        pthread_create(&tids[i], NULL, thread_func, &threads[i]);
    }

    sleep(1);
    schedule();

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tids[i], NULL);

    printf("\n");
    double total_wait = 0.0, total_tat = 0.0;
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Thread %d: waiting = %d, turnaround = %d\n",
               threads[i].id, threads[i].waiting_time, threads[i].turnaround_time);
        total_wait += threads[i].waiting_time;
        total_tat  += threads[i].turnaround_time;
    }
    printf("\nAverage waiting time = %.3f\n", total_wait / NUM_THREADS);
    printf("Average turnaround time = %.3f\n", total_tat / NUM_THREADS);

    return 0;
}
