# LINUX VERSION: 5.14-rc4
PWD := $(shell pwd) 
KVERSION := $(shell uname -r)
KERNEL_DIR = /home/kruztw/Downloads/linux/
#KERNEL_DIR = /usr/src/linux-headers-$(KVERSION)

MODULE_NAME = modulea
obj-m := $(MODULE_NAME).o

#KBUILD_EXTRA_SYMBOLS+=$(PWD)/Module.symvers
#export KBUILD_EXTRA_SYMBOLS

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	@rm .* modules.order $(MODULE_NAME).mod* *.dwo $(MODULE_NAME).o .simple.* > /dev/null 2>&1 || true
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
