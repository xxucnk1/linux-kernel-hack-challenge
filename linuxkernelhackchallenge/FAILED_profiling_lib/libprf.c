/*  
 *  libprf.c - A simple profiler library to send enable/disable OS commands to the profiler kernel 
 *  module and measure time (in ns) elapsed between start and stop functions.
 * 
 * 	NB! This library does not support multi-threaded applications
 */	
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "libprf.h"
#include "../kernel/km_os_ctrl/os_ctrl_common.h"

// Start and stop times
static struct timespec 		t_start, t_stop;

// 
// Start profiling
// 	 
//		Input
//			Interrupt_At_US: 	The L1 cache invalidation time delay (in ns)
//
// 		Return: 
// 			The start command execution status (one of enum libprf_ret_status{})
//
int
START_PROFILING( uint64_t Interrupt_At_US ){
	int status = LIBPRF_SUCCESS;
	FILE *prf_proc_file 		= NULL;
	unsigned char tx_buf[ OS_CTRL_START_CMD_LENGTH ];
	// Set pointer to pid field in tx buffer
	pid_t *p_id 				= (pid_t *)(&tx_buf[ OS_CTRL_TX_PID_OFFSET ]);
	// Set pointer to L1 inv delay field in tx buffer
	uint64_t *data 				= (uint64_t *)(&tx_buf[ OS_CTRL_TX_L1_INV_DELAY_NS_OFFSET ]);
	*p_id				 		= getpid(); 						// PID to profile
	tx_buf[ OS_CTRL_TX_CMD_OFFSET ]	= OS_CTRL_TASK_CMD_DISABLE_OS;	// Start cmd = OS_CTRL_TASK_CMD_DISABLE_OS			
	*data 						= Interrupt_At_US; 					// Interrupt delay (us)	
	// Open proc file in write mode
	if( !(prf_proc_file=fopen("/proc/os_ctrl","w")) ){
		perror( "libprf START_PROFILING fopen()" ); 
		status = LIBPRF_ERR_OPEN_FILE;
	}
	// Resize buffer
	else if( setvbuf(prf_proc_file,(char *)tx_buf,_IOFBF,sizeof(tx_buf)) != 0 ){
		perror( "libprf START_PROFILING setvbuf()" ); 
		status = LIBPRF_ERR_BUF_RESIZE;
	}
	// Write data to os_ctrl proc file
	else if( fwrite(tx_buf,sizeof(unsigned char),sizeof(tx_buf),prf_proc_file) != sizeof(tx_buf) ){
		perror( "libprf START_PROFILING fwrite()" ); 
		status = LIBPRF_ERR_WRITE;
	}
	// Make sure the buffer is flushed and sent to kernel module
	else if( fflush(prf_proc_file) == EOF ){
		perror( "libprf START_PROFILING fflush()" ); 
		status = LIBPRF_ERR_BUF_FLUSH;
	}
	// Close proc file
	else if( fclose(prf_proc_file) == EOF ){
		perror( "libprf START_PROFILING fclose()" ); 
		status = LIBPRF_ERR_CLOSE_FILE;
	}
	// Get the start cmd time stamp
	else if( clock_gettime(CLOCK_REALTIME,&t_start) < 0 ){
		perror( "libprf START_PROFILING clock_gettime()" ); 
		status = LIBPRF_ERR_GET_TIME;
	}
	// Return start command status
	return status;
}			

// 
// Stop profiling
// 	 
//		Input
//			Measured_Time_NS: 	Pointer to where to store the execution time
//
// 		Return: 
// 			The stop command execution status (one of enum libprf_ret_status{})
//
int
STOP_PROFILING( uint64_t *Measured_Time_NS ){
	// First get the stop cmd timestamp
	int err = (clock_gettime(CLOCK_REALTIME,&t_stop) < 0);
	if( err ){
		perror( "libprf STOP_PROFILING clock_gettime()" ); 
	}
	int status = LIBPRF_SUCCESS;
	FILE *prf_proc_file 		= NULL;
	unsigned char tx_buf[ OS_CTRL_STOP_CMD_LENGTH ];
	unsigned char rx_buf[ OS_CTRL_RX_LENGTH ];
	// Set pointer to pid field in tx buffer
	pid_t *p_id 				= (pid_t *)(&tx_buf[ OS_CTRL_TX_PID_OFFSET ]);
	*p_id				 		= getpid(); 								// PID to profile
	// Set cmd
	tx_buf[ OS_CTRL_TX_CMD_OFFSET ]	= OS_CTRL_TASK_CMD_ENABLE_OS;			// Stop cmd = OS_CTRL_TASK_CMD_ENABLE_OS				
	// Set pointer to L1 inv. time field in rx buffer
	uint64_t *pL1inv_dt 		= (uint64_t *)(&rx_buf[ OS_CTRL_RX_L1_INV_OFFSET ]);
	// Set pointer to the number of volontary context swithes between start and stop field in rx buffer
	unsigned char *pNvcsw		= &rx_buf[ OS_CTRL_RX_NVCSW_OFFSET ]; 		// Number of volontary constext switches
	// Set pointer to the number of involontary context swithes between start and stop field in rx buffer
	unsigned char *pNivcsw		= &rx_buf[ OS_CTRL_RX_NIVCSW_OFFSET ]; 		// Number of involontary constext switches
	// Reset measured time
	*Measured_Time_NS 	= 0; 
	// Open proc file in read and write mode
	if( !(prf_proc_file=fopen("/proc/os_ctrl","w+")) ){
		perror( "libprf STOP_PROFILING fopen()" ); 
		status = LIBPRF_ERR_OPEN_FILE;
	}
	// Resize buffer
	else if( setvbuf(prf_proc_file,(char *)tx_buf,_IOFBF,sizeof(tx_buf)) != 0 ){
		perror( "libprf STOP_PROFILING setvbuf()" ); 
		status = LIBPRF_ERR_BUF_RESIZE;
	}
	// Write data to os_ctrl proc file
	else if( fwrite(tx_buf,sizeof(unsigned char),sizeof(tx_buf),prf_proc_file) != sizeof(tx_buf) ){
		perror( "libprf STOP_PROFILING fwrite()" ); 
		status = LIBPRF_ERR_WRITE;
	}
	// Make sure the buffer is flushed and sent to kernel module
	else if( fflush(prf_proc_file) == EOF ){
		perror( "libprf STOP_PROFILING fflush()" ); 
		status = LIBPRF_ERR_BUF_FLUSH;
	}
	// Read answer and update *Measured_Time_NS if no error occurres
	if( fread(rx_buf,sizeof(unsigned char),sizeof(rx_buf),prf_proc_file) != sizeof(rx_buf) ) {
		perror( "libprf STOP_PROFILING fread()" ); 
		status = LIBPRF_ERR_READ;
	}
	// Close proc file
	else if( fclose(prf_proc_file) == EOF ){
		perror( "libprf START_PROFILING fclose()" ); 
		status = LIBPRF_ERR_CLOSE_FILE;
	}
	// Compute the execution time
	*Measured_Time_NS = (ELAPSED(t_start,t_stop) > *pL1inv_dt) ? (ELAPSED(t_start,t_stop) - *pL1inv_dt) : 0;
	// Return stop cmd status
	return ((status == LIBPRF_SUCCESS) && ((*Measured_Time_NS == 0) || (*pNvcsw != 0) || (*pNivcsw != 0))) ? LIBPRF_ERR_KM_BAD_VALUE : status;
}			

