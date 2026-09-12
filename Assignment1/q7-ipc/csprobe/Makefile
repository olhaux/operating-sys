# named csprobe.c so the parameter is at /sys/module/csprobe/parameters/target_pid
obj-m += csprobe.o

KDIR ?= /lib/modules/$(shell uname -r)/build

# CURDIR, not PWD: PWD is wrong when this runs through make -C

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
