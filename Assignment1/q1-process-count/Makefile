CC     = gcc
CFLAGS = -O0 -Wall -Wextra
# NOTE: keep this name <= 14 chars. Linux truncates a task's "comm" to 15
# characters, so a longer binary name breaks exact-match pgrep/pkill patterns.
TARGET = hw1-proc-count

all: $(TARGET)

$(TARGET): hw1-proc-count.c
	$(CC) $(CFLAGS) -o $@ $<

# Launch, wait 2s, count the processes, show the tree, then clean up.
run: $(TARGET)
	@./$(TARGET) & sleep 2; \
	 echo "process count: $$(pgrep -cx '$(TARGET)')"; \
	 pstree -p $$(pgrep -ox '$(TARGET)') | head -20; \
	 pkill -9 -x '$(TARGET)' || true

clean:
	rm -f $(TARGET)

.PHONY: all run clean
