obj-m := character-driver-module.o

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
KERNELDIR ?= /home/liam/dev/kernels/staging

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
