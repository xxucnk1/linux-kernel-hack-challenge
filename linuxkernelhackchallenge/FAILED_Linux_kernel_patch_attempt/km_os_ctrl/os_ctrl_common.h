/*  
 *  os_ctrl_common.h - Header file containing common things between the control OS kernel module and libprf
 * 
 * 	Communication protocol:
 * 
 * 		TX:
 * 				+ PID to start/stop profiling  							-> sizeof(pid_t) bytes
 * 				+ CMD (one of enum os_ctrl_task_commads{})				-> 1 byte
 * 				+ L1 inv. int delay in us (set to 0 if unused)			-> sizeof(uint64_t) = 8 bytes)
 * 				  NB: If CMD=OS_CTRL_TASK_CMD_ENABLE_OS then the L1 inv. int delay is not used
 * 
 * 		RX:
 * 				+ The L1 cache inv. time in ns 							-> sizeof(uint64_t) = 8bytes
 * 				+ The number of voluntary context switches			 	-> 1 byte
 * 				+ The number of involuntarily context switches 			-> 1 byte
 * 
 * 	
 */
#ifndef _OS_CTRL_COMMON_H_

	#define _OS_CTRL_COMMON_H_

	#define SEC2NANO 							1000000000L
	#define ELAPSED( t_start , t_stop ) 		(SEC2NANO *(t_stop.tv_sec - t_start.tv_sec) + (t_stop.tv_nsec - t_start.tv_nsec))

	// Start/Stop command lengths
	#define OS_CTRL_START_CMD_LENGTH 			(sizeof(pid_t) + 1 + sizeof(uint64_t))
	#define OS_CTRL_STOP_CMD_LENGTH 			(sizeof(pid_t) + 1)
	
	// Kernel module RX length
	#define OS_CTRL_RX_LENGTH 					(sizeof(uint64_t) + 2)

	// Kernel module TX data fields offsets in buffer
	#define OS_CTRL_TX_PID_OFFSET				0 						// The PID to profile offset value
	#define OS_CTRL_TX_CMD_OFFSET				sizeof(pid_t)			// The CMD to profile offset value
	#define OS_CTRL_TX_L1_INV_DELAY_NS_OFFSET	OS_CTRL_TX_CMD_OFFSET + 1 // The L1 inv. int delay in ns offset value

	// Kernel module RX data fields offsets in buffer
	#define OS_CTRL_RX_L1_INV_OFFSET			0 						// L1 invalidation time offset value (in ns)
	#define OS_CTRL_RX_NVCSW_OFFSET				sizeof(uint64_t)		// The number of voluntary context switches offset value
	#define OS_CTRL_RX_NIVCSW_OFFSET			OS_CTRL_RX_NVCSW_OFFSET + 1	// The number of involuntary context switches offset value
	
	// All control os kernel module commands available
	enum {
		OS_CTRL_TASK_CMD_DISABLE_OS 			= 0x00, 	// Used when starting a profile
		OS_CTRL_TASK_CMD_ENABLE_OS 				= 0x01, 	// Used when stopping a profile
		OS_CTRL_TASK_CMD_NONE 					= 0xFF, 	// No cmd
	} os_ctrl_task_commads;

#endif // _OS_CTRL_COMMON_H_ 
