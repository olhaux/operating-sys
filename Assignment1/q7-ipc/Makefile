CC     = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = hw1-ipc

# Big default for the csprobe sampling run (long enough to cat /proc/csprobe
# three times). Override for strace: make strace STRACE_ITERS=20000
ITERS        ?= 10000000
STRACE_ITERS ?= 20000

# 'all' deliberately builds ONLY the user-space program, so it still succeeds
# on a machine without kernel headers. Build the kernel module separately with
#     make module
# See the csprobe/ subdirectory.
all: $(TARGET)

$(TARGET): hw1-ipc.c
	$(CC) $(CFLAGS) -DITERS=$(ITERS) -o $@ $<

run: $(TARGET)
	./$(TARGET) 0

# ---- Q7.4 / Q7.5: the csprobe kernel module ----
# Requires linux-headers-$(uname -r). Loading it requires root.
module:
	$(MAKE) -C csprobe

# Tolerant: succeeds even without kernel headers, so 'make clean' never fails.
module-clean:
	-@$(MAKE) -C csprobe clean 2>/dev/null || true

# End-to-end Q7.4 + Q7.5 sequence. Needs sudo; run it interactively.
# Scripts are invoked through bash rather than ./script.sh: git on Windows
# (core.filemode=false) and most zip tools drop the executable bit, which
# would otherwise fail with 'Permission denied'.
verify: $(TARGET)
	bash ./verify-q7.sh

# Q7.2 -- small ITERS, or strace makes this take hours.
strace:
	$(CC) $(CFLAGS) -DITERS=$(STRACE_ITERS) -o $(TARGET)-strace hw1-ipc.c
	strace -f -e trace=read,write -c ./$(TARGET)-strace 0

# Q7.6 -- context-switch counts across several ITERS values.
ctxsw:
	bash ./measure-cs.sh 250000 500000 1000000

# NOTE: 'clean' must work on a machine with no kernel headers, so it does NOT
# depend on module-clean. Use 'make module-clean' for the .ko build products.
clean:
	rm -f $(TARGET) $(TARGET)-strace ipc-test ipc-strace ipc-cs \n	      Module.symvers modules.order .Module.symvers.cmd .modules.order.cmd

.PHONY: all run module module-clean verify strace ctxsw clean
