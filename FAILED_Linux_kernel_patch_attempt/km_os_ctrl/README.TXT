				OS CONTROL KERNEL MODULE

This kernel module can be used together with the profiling library in ../../profiling_lib to 
enable/disable parts of the OS to profile a user application using the library API.
This kernel module needs a modified freescale linux kernel to operate. 
A patch for kernel version 2.6.32 is available in ../scheduler_mod.  


INSTALLATION:
	To compile the kernel module run:
	> make clean all

	
USAGE:
	To use the kernel module, run (as root)
	> insmod os_ctrl.ko
	
	Once the kernel module is inserted, /proc/os_ctrl is created.
	The profiling tool library will use this file to communicate with the kernel module
	according to the communication protocol described below
	
	To remove the kernel module run (as root)
	> rmmod os_ctrl


COMMUNICATION PROTOCOL:
	To use the communication protocol offered by this kernel module, include the 'os_ctrl_common.h'
	header file in your code (if you wish to use it in other projects)
		#include "os_ctrl_common.h"
		
	+ START PROFILING COMMAND (disable OS kernel module cmd):
		- The PID to start profiling 									-> sizeof(pid_t) (4 bytes on the P1022DS PPC architchture) 	
		- The disable OS command 'OS_CTRL_TASK_CMD_DISABLE_OS = 0'		-> 1 byte
		- The L1 inv. interrupt delay in us (set to 0 if unused) 		-> sizeof(unit64_t) (8 bytes)
		
	+ STOP PROFILING COMMAND (enable OS kernel module cmd):
		- The PID to stop profiling 									-> sizeof(pid_t) (4 bytes on the P1022DS PPC architchture) 	
		- The enable OS command 'OS_CTRL_TASK_CMD_ENABLE_OS = 1'		-> 1 byte
		
	+ KERNEL MODULE DATA:
		- The L1 cache invalidation time in ns (0 on error)				-> sizeof(uint64_t) = 8bytes
 		- The number of volontary context switches			 			-> 1 byte
 		- The number of involontary context switches 					-> 1 byte
	
