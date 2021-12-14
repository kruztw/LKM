ifneq	($(KERNELRELEASE),)
obj-m	:= simple.o

else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)
default:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc user.c -o user -ggdb
	rm -r -f .tmp_versions .*.cmd *.o *.symvers modules* *.mod* built-in* 

load:
	insmod ./simple.ko
unload:
	rmmod simple || true
clean:
	rm simple.ko user

endif

