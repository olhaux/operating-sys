/* Q7: map N anonymous pages with mmap(), either normal pages (option 1)
 * or huge pages (option 2, pass "huge" as argv[2]). */
#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#define HAVE_RDTSC 1
#endif

static unsigned long long now_cycles(void)
{
#ifdef HAVE_RDTSC
    return __rdtsc();                       /* CPU timestamp counter = cycles */
#else
    struct timespec ts;                     /* fallback (e.g. ARM): nanoseconds */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s N [huge]\n", argv[0]);
        return 1;
    }
    int num_pages = atoi(argv[1]);
    int use_huge = (argc > 2 && strcmp(argv[2], "huge") == 0);
    int page_size = getpagesize();
    size_t length = (size_t)page_size * num_pages;

    printf("Allocating %d pages of %d bytes (%s)\n", num_pages, page_size,
           use_huge ? "option 2: huge pages" : "option 1: normal pages");

    char *addr;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    if (use_huge)
        flags |= MAP_HUGETLB;               /* option 2 */

    unsigned long long start = now_cycles();          /* start timer */

    addr = (char *)mmap(NULL, length, PROT_READ | PROT_WRITE, flags, -1, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    /* the code below updates the pages */
    char c = 'a';
    for (int i = 0; i < num_pages; i++) {
        addr[(size_t)i * page_size] = c;
        c++;
    }

    unsigned long long end = now_cycles();            /* end timer */
#ifdef HAVE_RDTSC
    printf("Elapsed time: %llu cycles\n", end - start);
#else
    printf("Elapsed time: %llu ns (no rdtsc on this CPU)\n", end - start);
#endif

    for (int i = 0; (i < num_pages && i < 16); i++)
        printf("%c ", addr[(size_t)i * page_size]);
    printf("\n");

    munmap(addr, length);
    return 0;
}
