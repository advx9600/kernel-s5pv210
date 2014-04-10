obj-m:=NVTtouch.o
KERNEL_DIR := /home/abc/Public/projects/shampoo_1311/kernel/kernel
PWD := $(shell pwd)  
all:  
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules  
clean:  
	rm *.o *.ko *.mod.c
