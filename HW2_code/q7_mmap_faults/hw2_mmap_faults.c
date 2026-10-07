// ./hw2_mmap_faults N for option 1, ./hw2_mmap_faults N huge for option 2
#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#define HAVE_RDTSC 1
#endif

// cycles from rdtsc on x86, ARM has no rdtsc so there we use nanoseconds
static unsigned long long now_cycles(void){
#ifdef HAVE_RDTSC
  return __rdtsc();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

int main(int argc, char** argv){

  unsigned long long start,end;

  if (argc < 2) {
    printf("usage: ./hw2_mmap_faults N [huge]\n");
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
    flags |= MAP_HUGETLB;

  // start the timer
  start = now_cycles();

  // option 1: num_pages anonymous pages, option 2: the same but with huge pages
  addr = (char*) mmap(NULL, length, PROT_READ | PROT_WRITE, flags, -1, 0);

  if (addr == MAP_FAILED) {
    if (use_huge && errno == ENOMEM)
      printf("mmap failed: no free huge pages, reserve some first with sudo sysctl -w vm.nr_hugepages=40\n");
    else
      perror("mmap");
    exit(1);
  }

  //the code below updates the pages
  char c = 'a';
  for(int i=0; i<num_pages; i++){
    addr[(size_t)i*page_size] = c;
    c ++;
  }

  // stop the timer
  end = now_cycles();

  // print the elapsed time in cycles (nanoseconds on ARM)
#ifdef HAVE_RDTSC
  printf("Elapsed time: %llu cycles\n", end - start);
#else
  printf("Elapsed time: %llu ns (no rdtsc on this CPU)\n", end - start);
#endif

  for(int i=0; (i<num_pages && i<16); i++){
    printf("%c ", addr[(size_t)i*page_size]);
  }
  printf("\n");

  munmap(addr, length);
  return 0;
}
