CC     = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = hw1-ipc

# override: make all ITERS=30000000
ITERS        ?= 10000000
STRACE_ITERS ?= 20000

all: $(TARGET) module

$(TARGET): hw1-ipc.c
	$(CC) $(CFLAGS) -DITERS=$(ITERS) -o $@ $<

run: $(TARGET)
	./$(TARGET) 0

# needs linux-headers-$(uname -r)
module:
	$(MAKE) -C csprobe

module-clean:
	-@$(MAKE) -C csprobe clean 2>/dev/null || true

# fewer iterations since strace is slow
strace:
	$(CC) $(CFLAGS) -DITERS=$(STRACE_ITERS) -o $(TARGET)-strace hw1-ipc.c
	strace -f -e trace=read,write -c ./$(TARGET)-strace 0

clean: module-clean
	rm -f $(TARGET) $(TARGET)-strace \
	      Module.symvers modules.order .Module.symvers.cmd .modules.order.cmd

.PHONY: all run module module-clean strace clean
