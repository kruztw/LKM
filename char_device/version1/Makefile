ifneq	($(KERNELRELEASE),)
obj-m	:= my_dev.o

else
KERNEL_DIR = /home/kruztw/Downloads/linux-5.10.1/
PWD	:= $(shell pwd)
default:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	rm -r -f .tmp_versions .*.cmd *.o *.symvers modules* *.mod* 

load:
	insmod ./my_dev.ko
	mknod /dev/my_dev c 66 0
unload:
	rm /dev/my_dev || true
	rmmod my_dev || true

endif

