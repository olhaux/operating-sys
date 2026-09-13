CC      = gcc
CFLAGS  = -O0 -Wall -Wextra
TARGETS = hw1-proc-count

all: $(TARGETS)

hw1-proc-count: hw1-proc-count.c
	$(CC) $(CFLAGS) -o $@ $<

run: hw1-proc-count
	./hw1-proc-count | wc -l

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
