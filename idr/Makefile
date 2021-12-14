obj-m:=simple.o
PWD := $(shell pwd) 
KVERSION := $(shell uname -r)
KERNEL_DIR = /home/kruztw/Downloads/linux/
#KERNEL_DIR = /usr/src/linux-headers-$(KVERSION)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	@rm .* modules.order Module.symvers *.mod* *.o .simple.* > /dev/null 2>&1 || true
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
