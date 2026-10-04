CC=gcc
CFLAGS=-Wall -O0
TIME=/usr/bin/time --verbose

all: hw2_mmap_faults

hw2_mmap_faults: hw2_mmap_faults.c
	$(CC) $(CFLAGS) -o $@ $<

run: all
	for n in 4096 8192 16384; do $(TIME) ./hw2_mmap_faults $$n; done
	for n in 4096 8192 16384; do $(TIME) ./hw2_mmap_faults $$n huge; done

clean:
	rm -f hw2_mmap_faults
