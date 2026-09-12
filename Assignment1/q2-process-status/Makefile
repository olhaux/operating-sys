CC     = gcc
CFLAGS = -O0 -Wall -Wextra
TARGET = hw1-process-status

all: $(TARGET)

$(TARGET): hw1-process-status.c
	$(CC) $(CFLAGS) -o $@ $<

# Run it, then read each process's State from /proc while the parent sleeps.
# Expect the parent in S (sleeping) and the child in Z (zombie).
run: $(TARGET)
	@./$(TARGET) & P=$$!; sleep 1; \
	 C=$$(ps -o pid= --ppid $$P | tr -d ' '); \
	 echo ""; echo "=== parent (pid $$P) ==="; grep -E 'Name|State|PPid' /proc/$$P/status; \
	 echo "=== child  (pid $$C) ==="; grep -E 'Name|State|PPid' /proc/$$C/status; \
	 wait

clean:
	rm -f $(TARGET)

.PHONY: all run clean
