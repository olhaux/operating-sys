CC     = gcc
CFLAGS = -O0 -Wall -Wextra
TARGET = hw1-process-status

all: $(TARGET)

$(TARGET): hw1-process-status.c
	$(CC) $(CFLAGS) -o $@ $<

# Run it, then inspect statuses while the parent is still sleeping.
run: $(TARGET)
	@./$(TARGET) & sleep 2; \
	 echo ""; echo "=== ps while parent sleeps (expect S+ parent, Z+ child) ==="; \
	 ps -o pid,ppid,stat,wchan:16,comm -C $(TARGET); \
	 echo ""; ps -ef | grep '[h]w1-process-status'; \
	 wait

clean:
	rm -f $(TARGET)

.PHONY: all run clean
