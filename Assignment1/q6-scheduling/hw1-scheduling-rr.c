/*
 * hw1-scheduling-rr.c  --  Q6.3: reduce thread 6's turnaround time to < 20
 *
 * WHY NOT PLAIN AGING?
 * If every waiting thread's priority improves at the SAME rate, the relative
 * order of the ready queue never changes, so thread 6 still runs last. Uniform
 * aging cannot fix this. Aging only works if it is non-uniform (e.g. applied
 * only past a starvation threshold).
 *
 * The clean fix is a TIME QUANTUM with round-robin dispatch: every runnable
 * thread gets the CPU once per sweep, so thread 6 (burst 1) finishes in the
 * very first sweep.
 *
 * NOTE: a quantum alone is NOT enough if the picker still sorts by priority --
 * thread 7 (prio 1) would simply win every quantum until it finished. The
 * dispatch order itself has to rotate.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 8
#define QUANTUM     1

typedef struct {
    int id, priority, burst_time, remaining;
    int start_time, completion_time, waiting_time, turnaround_time;
} thread_info_t;

thread_info_t threads[NUM_THREADS];

pthread_mutex_t sched_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  sched_cond = PTHREAD_COND_INITIALIZER;
int current_running_tid = -1;
int sim_clock = 0;
int last_run  = -1;

/* Round-robin: start scanning just past whoever ran last, wrapping around. */
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

        if (t->start_time < 0)
            t->start_time = sim_clock;

        /* Run at most one quantum, then yield -- this is PREEMPTIVE. */
        int slice = (t->remaining < QUANTUM) ? t->remaining : QUANTUM;
        printf("t=%2d: Thread %d (prio %d) runs %d unit(s)  [remaining %d -> %d]\n",
               sim_clock, t->id, t->priority, slice,
               t->remaining, t->remaining - slice);

        sim_clock   += slice;
        t->remaining -= slice;

        if (t->remaining == 0) {
            t->completion_time = sim_clock;
            t->turnaround_time = t->completion_time;
            t->waiting_time    = t->turnaround_time - t->burst_time;
            printf("      -> Thread %d COMPLETED at t=%d (turnaround %d)\n",
                   t->id, sim_clock, t->turnaround_time);
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
        threads[i].start_time = -1;
        pthread_create(&tids[i], NULL, thread_func, &threads[i]);
    }

    sleep(1);
    schedule();

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tids[i], NULL);

    printf("\n%-8s %-6s %-7s %-8s %-10s %-10s\n",
           "Thread", "Prio", "Burst", "Start", "Waiting", "Turnaround");
    double total_wait = 0.0, total_tat = 0.0;
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("%-8d %-6d %-7d %-8d %-10d %-10d\n",
               threads[i].id, threads[i].priority, threads[i].burst_time,
               threads[i].start_time, threads[i].waiting_time,
               threads[i].turnaround_time);
        total_wait += threads[i].waiting_time;
        total_tat  += threads[i].turnaround_time;
    }
    printf("\nQUANTUM = %d\n", QUANTUM);
    printf("Average waiting time    = %.3f\n", total_wait / NUM_THREADS);
    printf("Average turnaround time = %.3f\n",  total_tat  / NUM_THREADS);
    printf("Thread 6 turnaround     = %d  (target: < 20)\n",
           threads[6].turnaround_time);

    return 0;
}
