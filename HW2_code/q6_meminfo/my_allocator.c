/* Q6: allocate N pages of the system page size with malloc().
 * Build with -DINIT to also initialise (touch) the memory (Q6.3). */
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
    int page_size = getpagesize();          /* Q6.1: system-specific page size */
    size_t bytes = (size_t)n * page_size;

    printf("Page size: %d bytes, allocating %ld pages = %zu bytes\n",
           page_size, n, bytes);

    char *buf = malloc(bytes);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }

#ifdef INIT
    memset(buf, 1, bytes);                  /* Q6.3: initialise every byte */
    printf("Memory initialised\n");
#endif

    free(buf);
    return 0;
}
