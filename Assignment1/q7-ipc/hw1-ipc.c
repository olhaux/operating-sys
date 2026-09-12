/*
 * In this C program, two processes communicate a single byte back and forth through pipes.
 * In a separate file, you will modify a custom kernel module to measure context switches in kernel space.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

// override with -DITERS=n
#ifndef ITERS
#define ITERS 10000000
#endif

/* Pin the calling process to a single logical CPU. */
static void pin_to_cpu(int cpu) {
    if (cpu < 0)
        return;

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("sched_setaffinity");
        exit(1);
    }
}

int main(int argc, char **argv) {
    int cpu = 0;                        /* default pinning to CPU 0 */
    if (argc > 1)
        cpu = atoi(argv[1]);

    int p2c[2], c2p[2];
    /* create two pipes parent->child p2c, child->parent c2p */
    if (pipe(p2c) < 0) { perror("pipe p2c"); exit(1); }
    if (pipe(c2p) < 0) { perror("pipe c2p"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    char byte = 'x';

    if (pid == 0) {
        pin_to_cpu(cpu);

        /* Close the ends this process never uses */
        close(p2c[1]);
        close(c2p[0]);

        for (int i = 0; i < ITERS; i++) {
            /* read from a pipe */
            if (read(p2c[0], &byte, 1) != 1) {
                perror("child read");
                _exit(1);
            }
            /* write to a pipe */
            if (write(c2p[1], &byte, 1) != 1) {
                perror("child write");
                _exit(1);
            }
        }
        close(p2c[0]);
        close(c2p[1]);
        _exit(0);
    }
    else{
        pin_to_cpu(cpu);

        /* Close the ends this process never uses */
        close(p2c[0]);
        close(c2p[1]);

        printf("child pid    : %d\n", pid);
        printf("parent pid   : %d\n", getpid());
        printf("pinned to cpu: %d\n", cpu);
        fflush(stdout);

        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < ITERS; i++) {
            /* wake the child */
            if (write(p2c[1], &byte, 1) != 1) {
                perror("parent write");
                exit(1);
            }
            /* read blocks until the child answers */
            if (read(c2p[0], &byte, 1) != 1) {
                perror("parent read");
                exit(1);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);

        double ns = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);

        printf("iterations   : %d\n", ITERS);
        printf("total time   : %.3f ms\n", ns / 1e6);
        printf("avg time     : %.3f ns\n", ns / ITERS);

        close(p2c[1]);
        close(c2p[0]);
        wait(NULL);
    }

    return 0;
}
