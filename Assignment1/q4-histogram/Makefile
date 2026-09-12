CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGET = hw1-histogram

all: $(TARGET)

$(TARGET): hw1-histogram.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	@echo "===== ./$(TARGET) 3 800 16 ====="     && ./$(TARGET) 3 800 16
	@echo "===== ./$(TARGET) 3 80000 16 ====="   && ./$(TARGET) 3 80000 16
	@echo "===== ./$(TARGET) 3 8000000 16 =====" && ./$(TARGET) 3 8000000 16

clean:
	rm -f $(TARGET)

.PHONY: all run clean
