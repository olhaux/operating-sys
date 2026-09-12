/*
 * hw1-ipc.c  --  Q7: pipe ping-pong between parent and child
 *
 * Two processes bounce a single byte back and forth through two pipes,
 * ITERS times. Both are pinned to the same CPU so every exchange forces a
 * context switch.
 *
 * ITERS is overridable at compile time:
 *     gcc -DITERS=100000 ...
 * Use a SMALL value under strace (strace adds tens of microseconds per
 * syscall; 10,000,000 iterations = 20,000,000 syscalls = hours).
 * Use the LARGE default when sampling /proc/csprobe, so the run lasts long
 * enough to read the file three times.
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
    /* p2c: parent writes p2c[1] -> child reads p2c[0]
     * c2p: child  writes c2p[1] -> parent reads c2p[0] */
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

        /* The child READS from p2c and WRITES to c2p, so it never uses
         * p2c's write end or c2p's read end. Closing them matters:
         * a pipe only reports EOF when *every* copy of its write end is
         * closed, so a leaked descriptor here would stop the peer from ever
         * seeing the pipe close. It also stops the fd table leaking. */
        close(p2c[1]);
        close(c2p[0]);

        for (int i = 0; i < ITERS; i++) {
            /* read from the parent->child pipe (blocks until parent writes) */
            if (read(p2c[0], &byte, 1) != 1) {
                perror("child read");
                _exit(1);
            }
            /* write back on the child->parent pipe */
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

        /* The parent WRITES to p2c and READS from c2p, so it never uses
         * p2c's read end or c2p's write end. */
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
