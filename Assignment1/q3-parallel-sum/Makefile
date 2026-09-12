CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGET = hw1-parallel-sum

all: $(TARGET)

$(TARGET): hw1-parallel-sum.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET) 1
	./$(TARGET) 2
	./$(TARGET) 4

clean:
	rm -f $(TARGET)

.PHONY: all run clean
