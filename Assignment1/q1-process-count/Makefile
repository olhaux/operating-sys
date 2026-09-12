CC     = gcc
CFLAGS = -O0 -Wall -Wextra
# NOTE: keep this name <= 15 characters. ps -e shows each process's name cut
# to 15 characters, so a longer binary name would not match the grep below.
TARGET = hw1-proc-count

all: $(TARGET)

$(TARGET): hw1-proc-count.c
	$(CC) $(CFLAGS) -o $@ $<

# Launch, wait 1 s, then count the processes and show the tree with ps.
# ps -e prints only process names, so the grep itself is never counted.
# The processes exit on their own when their sleep(30) ends.
run: $(TARGET)
	@./$(TARGET) & sleep 1; \
	 echo "processes named $(TARGET): $$(ps -e | grep -c $(TARGET))"; \
	 ps -e --forest | grep $(TARGET) | head -20; \
	 echo "(they exit on their own when sleep(30) ends)"; wait

clean:
	rm -f $(TARGET)

.PHONY: all run clean
