ifneq	($(KERNELRELEASE),)
obj-m	:= simple.o

else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	rm -r -f .tmp_versions .*.cmd *.o *.symvers modules* *.mod* 

test: load
	tcpdump -i wlo1 -lvnn dst 192.168.0.86 -A

load:
	insmod ./simple.ko
unload:
	-rmmod simple
clean:
	rm ./simple.ko

endif

