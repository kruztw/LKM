KERNEL_DIR = /home/kruztw/Downloads/linux-5.10.1/
obj-m := simple.o
EXTRA_CFLAGS=-g -O0 	        
	
all:        
	make -C $(KERNEL_DIR) M=$(PWD) modules
	rm -r -f .tmp_versions .*.cmd *.o *.symvers modules* *.mod* 
clean:
	rm ./simple.ko
