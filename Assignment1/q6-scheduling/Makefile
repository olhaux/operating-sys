CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGETS = hw1-scheduling hw1-scheduling-rr

all: $(TARGETS)

hw1-scheduling: hw1-scheduling.c
	$(CC) $(CFLAGS) -o $@ $<

hw1-scheduling-rr: hw1-scheduling-rr.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGETS)
	./hw1-scheduling
	./hw1-scheduling-rr

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
