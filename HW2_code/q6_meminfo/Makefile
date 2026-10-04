CC=gcc
CFLAGS=-Wall -O0
N?=1024

all: my_allocator my_allocator_init

my_allocator: my_allocator.c
	$(CC) $(CFLAGS) -o $@ $<

my_allocator_init: my_allocator.c
	$(CC) $(CFLAGS) -DINIT -o $@ $<

run: all
	/usr/bin/time --verbose ./my_allocator $(N)
	/usr/bin/time --verbose ./my_allocator_init $(N)

clean:
	rm -f my_allocator my_allocator_init
