CC      = gcc
CFLAGS  = -O0 -Wall -Wextra
TARGETS = hw1-proc-count hw1-proc-sleep

all: $(TARGETS)

hw1-proc-count: hw1-proc-count.c
	$(CC) $(CFLAGS) -o $@ $<

hw1-proc-sleep: hw1-proc-sleep.c
	$(CC) $(CFLAGS) -o $@ $<

# Q1.2: every process prints its PID once, so counting the lines counts the
# processes. (hw1-proc-sleep is for the second check, watched with top.)
run: hw1-proc-count
	./hw1-proc-count | wc -l

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
