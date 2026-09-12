CC     = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = hw1-ipc

# Big default for the csprobe runs, long enough to read /proc/csprobe several
# times. Override on the command line:  make clean && make all ITERS=30000000
ITERS        ?= 10000000
STRACE_ITERS ?= 20000

# 'all' deliberately builds ONLY the user-space program, so it still succeeds
# on a machine without kernel headers. Build the kernel module separately with
#     make module
# (see the csprobe/ subdirectory).
all: $(TARGET)

$(TARGET): hw1-ipc.c
	$(CC) $(CFLAGS) -DITERS=$(ITERS) -o $@ $<

run: $(TARGET)
	./$(TARGET) 0

# ---- Q7.4 / Q7.5: the csprobe kernel module ----
# Requires linux-headers-$(uname -r). Loading it requires root.
module:
	$(MAKE) -C csprobe

# Tolerant: succeeds even without kernel headers.
module-clean:
	-@$(MAKE) -C csprobe clean 2>/dev/null || true

# Q7.2 -- small ITERS, or strace makes this take hours.
strace:
	$(CC) $(CFLAGS) -DITERS=$(STRACE_ITERS) -o $(TARGET)-strace hw1-ipc.c
	strace -f -e trace=read,write -c ./$(TARGET)-strace 0

# NOTE: 'clean' must work on a machine with no kernel headers, so it does NOT
# depend on module-clean. Use 'make module-clean' for the .ko build products.
clean:
	rm -f $(TARGET) $(TARGET)-strace \
	      Module.symvers modules.order .Module.symvers.cmd .modules.order.cmd

.PHONY: all run module module-clean strace clean
