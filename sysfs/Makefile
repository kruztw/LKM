PWD := $(shell pwd) 
KVERSION := $(shell uname -r)
KERNEL_DIR = /usr/src/linux-headers-$(KVERSION)

MODULE_NAME = simple
obj-m := $(MODULE_NAME).o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	@rm modules.order Module.symvers $(MODULE_NAME).mod* $(MODULE_NAME).o > /dev/null 2>&1 || true
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
