# Kbuild makefile for the csprobe kernel module.
# The module is named csprobe.ko, so its parameters live at
#   /sys/module/csprobe/parameters/target_pid
# (Keep the filename csprobe.c -- naming it hw1-csprobe.c would produce
#  /sys/module/hw1_csprobe/... and the in-code instructions would be wrong.)
obj-m += csprobe.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

run: all
	sudo insmod csprobe.ko
	@echo "loaded. set a pid with:"
	@echo "  echo <pid> | sudo tee /sys/module/csprobe/parameters/target_pid"
	@echo "  cat /proc/csprobe"

unload:
	sudo rmmod csprobe

.PHONY: all clean run unload
