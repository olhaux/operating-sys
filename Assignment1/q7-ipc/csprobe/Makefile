# Kbuild makefile for the csprobe kernel module.
# The module is named csprobe.ko, so its parameters live at
#   /sys/module/csprobe/parameters/target_pid
# (Keep the filename csprobe.c -- naming it hw1-csprobe.c would produce
#  /sys/module/hw1_csprobe/... and the in-code instructions would be wrong.)
obj-m += csprobe.o

KDIR ?= /lib/modules/$(shell uname -r)/build

# M= must be $(CURDIR), not $(PWD). PWD comes from the calling shell and is
# NOT updated by 'make -C csprobe', so when this Makefile is reached through
# 'make module' from q7-ipc/, $(PWD) still names the parent directory and
# kbuild silently builds nothing. CURDIR is set by make itself.

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

run: all
	sudo insmod csprobe.ko
	@echo "loaded. set a pid with:"
	@echo "  echo <pid> | sudo tee /sys/module/csprobe/parameters/target_pid"
	@echo "  cat /proc/csprobe"

unload:
	sudo rmmod csprobe

.PHONY: all clean run unload
