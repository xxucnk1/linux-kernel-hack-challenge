					
				PROFILER LIBRARY

This library contains the start and stop commands used to profile a given user application.
This library needs the kernel module (in ../kernel/km_os_ctrl) and a modified vanilla Linux 
kernel 2.6.32 (see patch in ../kernel/scheduler_mod).


INSTALLATION:
	To compile the library run:
	> make clean all
	
	The static and shared versions of the library, libprf.a and libprf.so respectively, are then compiled

	
USAGE:
	First, include the profiling library header file 'libprf.h' in your application
		+ #include "libprf.h"
	
	Then, embed the start function 'START_PROFILING' in the beginning of the code you would 
	like to profile.
	
	The start function START_PROFILING takes the L1 invalidation time interrupt delay (in us) as only argument. 
	Set this input argument to 0 if you do not wish to use this feature. 

	Then at last, embed the stop function STOP_PROFILING at the end of the code you wish to profile.
	
	The stop function STOP_PROFILING takes a pointer to where to store the execution time (in ns) for 
	the code you wish to profile.

	
ERROR HANDLING:
	The start and stop functions described above return the following error codes:	
		LIBPRF_SUCCESS 			= 0		-> Success - everything is ok
		LIBPRF_ERR_OPEN_FILE 	= 1 	-> An error has occurred while opening file
		LIBPRF_ERR_CLOSE_FILE 	= 2		-> An error has occurred while closing file
		LIBPRF_ERR_READ 		= 3		-> An error has occurred while reading from file	
		LIBPRF_ERR_WRITE 		= 4 	-> An error has occurred while writing to file
		LIBPRF_ERR_GET_TIME 	= 5 	-> An error has occurred while getting a time stamp
		LIBPRF_ERR_BUF_RESIZE 	= 6 	-> An error has occurred while resizing a file stream buffer
		LIBPRF_ERR_BUF_FLUSH 	= 7 	-> An error has occurred while flushing stream buffer				
		LIBPRF_ERR_UNKNOWN 		= 8 	-> Unknown error
		LIBPRF_ERR_KM_BAD_VALUE = 255 	-> Data received from kernel module is invalid (Time between start/stop is 0 or voluntary/involuntary context switches have occurred)
	


