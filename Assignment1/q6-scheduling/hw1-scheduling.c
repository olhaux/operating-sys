/*
 * hw1-scheduling.c  --  Q6: non-preemptive priority scheduling simulation
 *
 * NOTE ON THE SKELETON: the original pick_next() had an extra closing brace,
 * which put "return best;" outside the function body -- the file did not
 * compile. That is fixed here.
 *
 * All 8 threads "arrive" at time 0. A global lock + condition variable let
 * exactly one thread be "on CPU" at a time. sim_clock is the simulated clock,
 * advanced by the burst of whichever thread just ran.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_THREADS 8

typedef struct {
    int id;
    int priority;      // lower is more important
    int burst_time;    // CPU burst time
    int remaining;     // remaining time
    int start_time;    // when it first got the CPU
    int completion_time;
    int waiting_time;
    int turnaround_time;
} thread_info_t;

thread_info_t threads[NUM_THREADS];

// Global scheduler lock: only one thread "on CPU" at a time
pthread_mutex_t sched_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  sched_cond = PTHREAD_COND_INITIALIZER;
int current_running_tid = -1;   // the id of thread allowed to run
int sim_clock = 0;              // simulated time

/* TODO 1 (completed): choose the runnable thread with the highest priority
 * (numerically lowest .priority); break ties by lowest id. */
int pick_next() {
    int best = -1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (threads[i].remaining > 0) {
            if (best == -1 ||
                threads[i].priority <  threads[best].priority ||
               (threads[i].priority == threads[best].priority &&
                threads[i].id       <  threads[best].id)) {
                best = i;
            }
        }
    }
    return best;
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

        /* This thread now holds the CPU. Because it runs its ENTIRE remaining
         * burst before schedule() is called again, this is NON-PREEMPTIVE. */
        t->start_time = sim_clock;
        printf("t=%2d: Thread %d (prio %d) runs for %d units\n",
               sim_clock, t->id, t->priority, t->remaining);

        sim_clock += t->remaining;
        t->completion_time = sim_clock;
        t->turnaround_time = t->completion_time;              /* arrival = 0 */
        t->waiting_time    = t->turnaround_time - t->burst_time;
        t->remaining = 0;

        pthread_mutex_unlock(&sched_lock);

        schedule();  // pick the next thread
    }
    return NULL;
}

int main() {
    pthread_t tids[NUM_THREADS];
    int prios[]  = {3, 3, 2, 2, 2, 6, 7, 1}; // lower has higher priority
    int bursts[] = {7, 6, 2, 8, 7, 1, 1, 9};

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].id = i;
        threads[i].priority = prios[i];
        threads[i].burst_time = threads[i].remaining = bursts[i];
        threads[i].start_time = -1;
        pthread_create(&tids[i], NULL, thread_func, &threads[i]);
    }

    sleep(1);
    schedule();     // kick off scheduling

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tids[i], NULL);

    /* TODO 2 (completed): print each thread's waiting and turnaround time. */
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

    /* TODO 3 (completed): print the averages. */
    printf("\nAverage waiting time    = %.3f\n", total_wait / NUM_THREADS);
    printf("Average turnaround time = %.3f\n",  total_tat  / NUM_THREADS);
    printf("Thread 6 turnaround     = %d\n", threads[6].turnaround_time);

    return 0;
}
