// allocates N pages with malloc(), build with -DINIT to also write to them
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s N\n", argv[0]);
        return 1;
    }
    long n = atol(argv[1]);
    int page_size = getpagesize(); // system page size
    size_t bytes = (size_t)n * page_size;

    printf("Page size: %d bytes, allocating %ld pages = %zu bytes\n",
           page_size, n, bytes);

    char *buf = malloc(bytes);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }

#ifdef INIT
    memset(buf, 1, bytes); // touch every page
    printf("Memory initialised\n");
#endif

    free(buf);
    return 0;
}
