 		
				PATCHING THE LINUX KERNEL


The scheduling function of the vanilla Linux kernel needs to be modified in order to force 
the scheduler, for a given period of time, to run a user space task/thread uninterrupted 
by other tasks (i.e some critical kernel tasks/threads). 

The profiling and its kernel module will not work without this patch.

The following patch is made for linux version 2.6.32 and might not work on other versions.

To apply the patch run the following command (as root):
	> cp sched_orig.c /usr/src/linux/kernel/sched.c 	(always use original file when patching)
	> patch /usr/src/linux/kernel/sched.c sched_patch

You'll need to recompile your kernel after applying the patch (as root)
	> cd /usr/src/linux
	> make uImage
	> cp arch/powerpc/boot/uImge /boot/uImge.new
	> reboot

Interrupt the U-Boot loader and run the following commands:
	> setenv bootargs root=/dev/sda3 rw console=ttyS0, 115200
	> sata
	> ext2load sata 0:3 1000000 /boot/uImage.new
	> ext2load sata 0:3 c00000 /boot/p1022ds.dtb
	> bootm 1000000 - c00000
	
FILE INFORMATION:
	README.TXT 				This readme file
	sched_orig.c 			The original sched.c (should always keep a copy of this)
	sched_mod.c 			The modified sched.c
	sched_patch 			The patch to use (generated by 'diff -Nur sched_orig.c sched_mod.c > sched_patch')
