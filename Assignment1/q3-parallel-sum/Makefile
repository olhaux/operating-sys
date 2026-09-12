CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGET = hw1-parallel-sum

all: $(TARGET)

$(TARGET): hw1-parallel-sum.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	@echo "===== 1 thread ====="  && ./$(TARGET) 1
	@echo "===== 2 threads =====" && ./$(TARGET) 2
	@echo "===== 4 threads =====" && ./$(TARGET) 4

clean:
	rm -f $(TARGET)

.PHONY: all run clean
