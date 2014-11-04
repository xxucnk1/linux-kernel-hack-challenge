/*  
 *  libprf.h - Header file for a simple profiler library to send enable/disable OS commands to the profiler kernel 
 *  module and measure time (in ns) elapsed between start and stop functions.
 * 
 * 	NB! This library does not support multi-threaded applications
 */
#ifndef _LIBPRF_
	#define _LIBPRF_

	#include <stdint.h>

	// All libprf execution statuses
	enum {
		LIBPRF_SUCCESS, 						// Success - everything is ok
		LIBPRF_ERR_OPEN_FILE,	 				// An error has occurred while opening file
		LIBPRF_ERR_CLOSE_FILE, 					// An error has occurred while closing file
		LIBPRF_ERR_READ, 						// An error has occurred while reading from file	
		LIBPRF_ERR_WRITE,						// An error has occurred while writing to file
		LIBPRF_ERR_GET_TIME,					// An error has occurred while getting a time stamp
		LIBPRF_ERR_BUF_RESIZE,					// An error has occurred while resizing a file stream buffer
		LIBPRF_ERR_BUF_FLUSH,					// An error has occurred while flushing stream buffer				
		LIBPRF_ERR_UNKNOWN, 					// Unknown error
		LIBPRF_ERR_KM_BAD_VALUE = 0xFF, 		// Data received from kernel module is invalid (Time between start/stop is 0 or voluntary/involuntary context switches have occurred)
	} libprf_ret_status;


	// 
	// Start profiling
	// 	 
	//		Input
	//			Interrupt_At_US: 	The L1 cache invalidation interrupt time delay (in us)
	//
	// 		Return: 
	// 			The start command execution status (one of enum libprf_ret_status{})
	//
	extern int 		START_PROFILING( uint64_t Interrupt_At_US );

	// 
	// Stop profiling
	// 	 
	//		Input
	//			Measured_Time_NS: 	Pointer to where to store the execution time (in ns)
	//
	// 		Return: 
	// 			The stop command execution status (one of enum libprf_ret_status{})
	//
	extern int 		STOP_PROFILING( uint64_t *Measured_Time_NS );
	
#endif // _LIBPRF_
