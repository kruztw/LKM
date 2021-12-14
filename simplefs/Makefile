# LINUX VERSION: 5.4.0
# 不要在本機跑會壞掉 !!!
# 用 kernel pwn 的 qemu 來實驗
# simple.c 還不夠簡短, 待完成

PWD := $(shell pwd) 
KVERSION := $(shell uname -r)
KERNEL_DIR = /home/kruztw/Downloads/linux/
#KERNEL_DIR = /usr/src/linux-headers-$(KVERSION)

MODULE_NAME = simple
obj-m := $(MODULE_NAME).o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	@rm modules.order Module.symvers $(MODULE_NAME).mod* $(MODULE_NAME).o .simple.* > /dev/null 2>&1 || true
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

mkfs:
	gcc mkfs.c -o mkfs
	dd if=/dev/zero of=image bs=4096 count=16
	./mkfs image
	
