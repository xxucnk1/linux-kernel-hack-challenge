obj-m += os_ctrl.o

#KERNELDIR = /usr/src/linux-$(shell uname -r)
KERNELDIR = /usr/src/linux

all:
	@echo "Compiling kernel module os_ctrl.ko"
	@make -C $(KERNELDIR) M=$(PWD) modules

clean:
	@echo "Cleaning kernel module"
	@make -C $(KERNELDIR) M=$(PWD) clean
