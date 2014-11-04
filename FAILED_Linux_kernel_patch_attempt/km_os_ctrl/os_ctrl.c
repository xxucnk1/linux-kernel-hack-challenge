/*  
 *  os_ctrl.c - Control OS kernel module
 */
#include <linux/module.h>						// Needed by all modules
#include <linux/kernel.h>						// Needed for KERN_INFO
#include <linux/rcupdate.h> 					// Needed to lock/unlock scheduler when handling critical data
#include <linux/proc_fs.h>						// Needed for  proc fs
#include <linux/pid.h>							// Needed for find_vpid()
#include <linux/sched.h>						// Needed for reading cputimes from the linux scheduler
#include <linux/interrupt.h>					// Needed for handling interrupts
#include <asm/uaccess.h>						// Needed for copy_from_user (memory is segmented - A pointer points to a buffer segment not whole buffer)
#include <linux/hrtimer.h> 						// Needed for high resolution timer (to time application interrupt)
#include <linux/ktime.h> 						// Needed for high resolution timer (to time application interrupt)
#include <linux/time.h>							// Needed for computing the time taken by handlers
#include <linux/slab.h> 						// Needed for kmalloc and kfree
#include <linux/namei.h> 						// Needed for custom_sys_open() handler
#include "os_ctrl_common.h" 					// Header file constaining common things between the control OS kernel module and libprf

// MODULE_LICENSE("COPYRIGHT SAAB GROUP AB - All rights reserved");
MODULE_LICENSE("GPL"); 	// OBS! Must be GPL to be able to use find_vpid() and some other kernel routines.
MODULE_AUTHOR("NN - Copyright SAAB GROUP AB - All rights reserved");
MODULE_DESCRIPTION("A task profiling tool");

#define MODULE_NAME			 				"os_ctrl"
#define PROCFS_MAX_SIZE 					13
#define KILL_APPLICATION_DELAY_S			10
#define SCHED_RESET_DELAY_NS				900000L
#define SCRATCH_SIZE 						49152      // 12 * 128 * 32

#define IRQ_ENABLE() 						{ for( n=0 ; !irq_enabled && (n<(int)(sizeof(irq_no)/sizeof(unsigned char))) ; n++ ) 	{ enable_irq( irq_no[n] ); } 	irq_enabled = 1; }
#define IRQ_DISABLE() 						{ for( n=0 ; irq_enabled && (n<(int)(sizeof(irq_no)/sizeof(unsigned char))) ; n++ ) 	{ disable_irq( irq_no[n] ); } 	irq_enabled = 0; }

// Force scheduler to run task with pid p_id until reset (p_id = 0)
extern void set_pid_force_run( const pid_t p_id );

// Set OS types
enum {
	OS_CTRL_OS_DISABLE,	
	OS_CTRL_OS_ENABLE,
} os_ctrl_set_os_type;

// Process id to profile
static pid_t 					p_id = 0;

// OS ctrl command
static unsigned char			cmd = OS_CTRL_TASK_CMD_NONE; 

// OS ctrl pocfs I/O file
static struct proc_dir_entry 	*os_ctrl_proc_file = NULL;

// Invalidate L1 cache timer
static struct hrtimer 			invalidate_L1_cache_timer;

// Kill application timer
static struct hrtimer 			kill_application_timer;

// Reset scheduler timer
static struct hrtimer 			sched_reset_timer;

// Invalidate L1 cache timer interrupt delay
static ktime_t					invalidate_L1_cache_timer_int_delay;

// Kill application interrupt delay
static ktime_t					kill_application_timer_int_delay;

// Reset scheduler interrupt delay
static ktime_t					sched_reset_timer_int_delay;

// Procfs I/O buffers
static char 					os_ctrl_input_buffer[ PROCFS_MAX_SIZE ];

// The interrupt signals to send to the application
static struct siginfo 			kill_app_signal;

// L1 invalidation start and end times
struct timespec 				t_zero; 			// Initialized in init_module()
static struct timespec 			t_L1_inv_start, t_L1_inv_end;

// The number of volontary and involontary context-switches done by the application between the start and stop commands
static unsigned long		 	d_nvcsw, d_nivcsw;

// Semaphore holding the profile ready flag (=1 profile ready to be read)
static unsigned char 			prf_ready = 0;

// IRQs enabled flag
static unsigned char 			irq_enabled = 1;

// Invalidate L1 instruction cache
void 
invalidate_L1_instruction_cache( void ){
	asm volatile("mfspr 3, 1011;      /* Read L1CSR1 to R3              */\
                ori   4, 3, 0x0002; /* R4 as R3 but with ICFI bit set */\
                mtspr 1011, 4;      /* Invalidate by writing ICFI bit */\
                mtspr 1011, 3;      /* Restore L1CSR1                 */\
                isync;"
                : /* no output */
                : /* no input */
                : "3", "4", "memory" /* GPR3 and GPR4 clobbered */);
}

// Flush and invalidate L1 data cache
void 
flush_and_invalidate_L1_data_cache( void ){
	void* scratchref = NULL;
	void* scratchend = NULL;
	scratchref = kmalloc(SCRATCH_SIZE, GFP_KERNEL);
	if (scratchref) {
		scratchend = scratchref + SCRATCH_SIZE;
		asm volatile(
		 "mfmsr 6;            /* Disable interrupts: Store MSR in R6          */\
		  lis   5, 0xfffd;    /*   Create mask in R5, high bytes              */\
		  ori   5, 5, 0x6fff; /*   Create mask in R5, low bytes               */\
		  and   5, 5, 6;      /*   Get original contents and clear CE, EE, ME */\
		  mtmsr 5;            /*   Write to MSR                               */\
		  isync;              /*                                              */\
		  mfspr 7, 1008;      /* Enable flush assist: Store HID0 in R7        */\
		  ori   5, 7, 0x0040; /*   Copy to R5 while setting DCFA bit          */\
		  mtspr 1008, 5;      /*   Write DCFA bit to HID0                     */\
		  msync;              /*                                              */\
		  isync;              /*                                              */\
		  1:;                 /* Write dummy data to cache:                   */\
		  dcbz  0,  %0;       /*   Write                                      */\
		  addi  %0, %0, 32;   /*   Increment ref pointer by one cache line    */\
		  cmpw  %0, %1;       /*   Compare to end pointer                     */\
		  blt   1b;           /*   Branch if less                             */\
		  mfspr 3, 1010;      /* Flash invalidate: Read L1CSR0 to R3          */\
		  ori   3, 3, 0x0002; /*   Set CFI bit in R3                          */\
		  msync;              /*                                              */\
		  isync;              /*                                              */\
		  mtspr 1010, 3;      /*   Write CFI bit to L1CSR0                    */\
		  isync;              /*                                              */\
		  mtspr 1008, 7;      /* Disable flush assist: Restore HID0           */\
		  mtmsr 6;            /* Enable interrupt: Restore MSR                */\
		  isync;"
		  : /* no output */
		  : "r" (scratchref), "r" (scratchend) /* input */
		  : "3", "5", "6", "7", "cc", "memory" /* clobbered */);
      
      kfree(scratchref);
	}
	else { printk( KERN_ALERT "%s ERROR: MEM not allocated!\n" , MODULE_NAME ); }
}

// List of all hw interrupt sources available (OBS! If you make some hardware changes, pleas update this array - run 'cat /proc/interrupts' to list all interrupts available on your machine) 
const unsigned char irq_no[] = {
	 17,       // eth0_g1_tx
	 18,       // eth0_g1_rx
	 20,       // fsldma-channel
	 21,       // fsldma-channel
	 22,       // fsldma-channel
	 23,       // fsldma-channel
	 24,       // eth0_g1_er
	 28,       // ehci_hcd:usb1
	 29,       // eth0_g0_tx
	 30,       // eth0_g0_rx
	 34,       // eth0_g0_er
	 41,       // fsl-sata
	 42,       // serial
	 43,       // i2c-mpc, i2c-mpc
	 45,       // talitos
	 58,       // talitos
	 72,       // mmc0
	 74,       // sl-sata
	 76,       // fsldma-channel
	 77,       // fsldma-channel
	 78,       // fsldma-channel
	 79,       // fsldma-channel
	247,       // mpic timer 0
	251,       // ipi call function
	252,       // ipi reschedule
	253,       // ipi call function single
};

// Reset all counters
void
reset_cntrs( void ){
	cmd 							= OS_CTRL_TASK_CMD_NONE;
	d_nvcsw	= d_nivcsw 				= 0;
	t_L1_inv_start = t_L1_inv_end 	= t_zero;		
}

// Invalidate L1 cache timer interrupt handler
enum hrtimer_restart
invalidate_L1_cache_interrupt_handler( \
				struct hrtimer 				*timer \
			){
	// Start counting the time we will be in here (use raw monotonic clock to avoid problems caused by potential NPT daemons during this measurement)
	getrawmonotonic( &t_L1_inv_start );
	// Invalidate L1 instruction and data caches
	invalidate_L1_instruction_cache();
	flush_and_invalidate_L1_data_cache();
	// Stop counting the time we were in here
	getrawmonotonic( &t_L1_inv_end );
	// Disable timer
	return HRTIMER_NORESTART;
}

// Kill application timer interrupt handler
enum hrtimer_restart
kill_application_interrupt_handler( \
				struct hrtimer 				*timer \
			){
	int n;
	struct pid *pApp_id;
	struct task_struct *pTask = NULL; 			
	// Always enable OS
	set_pid_force_run( 0 );
	// Enable IRQs
	IRQ_ENABLE();
	// Inform user
	printk( KERN_INFO "%s: [PID %u] Application timed out after %llu s\n" , 													MODULE_NAME , p_id , (u64)KILL_APPLICATION_DELAY_S );
	// Cancel L1 invalidate cache timer (if it is a start/stop cmd only)
	if( invalidate_L1_cache_timer.state != HRTIMER_STATE_INACTIVE ){ 
		printk( KERN_INFO "%s: [PID %u] Stopping L1 invalidation cache timer\n", 												MODULE_NAME , p_id );
		hrtimer_cancel( &invalidate_L1_cache_timer ); 
	}
	if( !p_id ) { printk( KERN_INFO "%s: [PID %d] Will NOT kill the init process, did you start profiling?\n" ,		 			MODULE_NAME , p_id ); return HRTIMER_NORESTART; }
	// Get the application task pointer from the scheduler (if the application is currently running then no need to look into the stack - spare some cpu cycles) otherwise 
	if( current->pid == p_id ) 																{ pTask = current; }
	else if( !(pApp_id=find_vpid((int)p_id)) || !(pTask=pid_task(pApp_id,PIDTYPE_PID)) ) 	{ printk( KERN_ALERT "%s ERROR: [PID %u] Unable to find task in scheduler\n" , 						MODULE_NAME , p_id ); return HRTIMER_NORESTART; }
	// Send the task a SIGKILL
	if( send_sig_info(SIGKILL,&kill_app_signal,pTask) < 0 ) 								{ printk( KERN_ALERT "%s ERROR: [PID %u] Unable to send signal SIGKILL to application\n" , 			MODULE_NAME , p_id ); }
	else 																					{ printk( KERN_INFO "%s: [PID %u] Sent a SIGKILL signal to application\n" , 						MODULE_NAME , p_id ); } 	
	// Disable timer
	return HRTIMER_NORESTART;
}

// Reset scheduler timer interrupt handler
enum hrtimer_restart
sched_reset_interrupt_handler( \
				struct hrtimer 				*timer \
			){
	int n;
	// Always enable OS
	set_pid_force_run( 0 );
	// Enable IRQs
	IRQ_ENABLE();
	// Disable timer
	return HRTIMER_NORESTART;
}

// Enable/Disable parts of the OS
void
set_os( \
				struct task_struct			*pTask, \
				const unsigned char 		enable \
	){
	static unsigned char 	last_enable = 0xFF;
	int n;
	// Cancel reset scheduler time-out timer
	if( (enable != last_enable) && (sched_reset_timer.state != HRTIMER_STATE_INACTIVE) ){ 
		hrtimer_cancel( &sched_reset_timer ); 
	}
	// Cancel application time-out timer
	if( (enable != last_enable) && (kill_application_timer.state != HRTIMER_STATE_INACTIVE) ){ 
		hrtimer_cancel( &kill_application_timer ); 
	}
	// Handle request
	if( enable != last_enable ) {
		switch( enable ){
			// Disable OS
			case OS_CTRL_OS_DISABLE:
				// Start kill application timer (just in case)
				kill_application_timer_int_delay	= ktime_set( KILL_APPLICATION_DELAY_S , 0 );
				hrtimer_init( &kill_application_timer , CLOCK_MONOTONIC, HRTIMER_MODE_REL );
				kill_application_timer.function 	= &kill_application_interrupt_handler;
				hrtimer_start( &kill_application_timer , kill_application_timer_int_delay , HRTIMER_MODE_REL );
				// Start scheduler reset timer (just in case)
				sched_reset_timer_int_delay	= ktime_set( 0 , SCHED_RESET_DELAY_NS );
				hrtimer_init( &sched_reset_timer , CLOCK_MONOTONIC, HRTIMER_MODE_REL );
				sched_reset_timer.function 	= &sched_reset_interrupt_handler;
				hrtimer_start( &sched_reset_timer , sched_reset_timer_int_delay , HRTIMER_MODE_REL );
				// Disable IRQs
				IRQ_DISABLE();
				// Force scheduler to run the application and nothing else until it's reset (OBS! Needs modified kernel - see schedule() in kernel/sched.c)
				if( pTask != NULL ) { set_pid_force_run( pTask->pid ); }
				break;
			// Default: Enable OS always
			case OS_CTRL_OS_ENABLE:
			default:
				// Enable IRQs
				IRQ_ENABLE();
				// Reset scheduler
				set_pid_force_run( 0 );
				break;
		}
	}
	// Save enable state for next time
	last_enable 	= enable;
}

// This function is called when the /proc/os_ctrl file is read by user (make sure only 1 user at the time)
int
on_read_handler( \
				char 						*buffer, \
				char 						**buffer_location, \
				off_t 						offset, \
				int 						buffer_length, \
				int 						*eof, \
				void 						*data \
			){
	int n;
	u64 *pOut 						= (u64 *)buffer;
	unsigned char *pNvcsw			= (unsigned char *)(&buffer[ 8 ]);
	unsigned char *pNivcsw			= (unsigned char *)(&buffer[ 9 ]);
	static unsigned char last_cmd 	= OS_CTRL_TASK_CMD_NONE;
	// Always enable OS
	set_pid_force_run( 0 );
	// Enable IRQs
	IRQ_ENABLE();
	// Cancel scheduler reset timer
	if( sched_reset_timer.state != HRTIMER_STATE_INACTIVE ){ 
		hrtimer_cancel( &sched_reset_timer ); 
	}
	// Cancel kill application timer
	if( kill_application_timer.state != HRTIMER_STATE_INACTIVE ){ 
		hrtimer_cancel( &kill_application_timer ); 
	}
	// Cancel L1 invalidate cache timer (if it is a start/stop cmd only)
	if( invalidate_L1_cache_timer.state != HRTIMER_STATE_INACTIVE ){ 
		hrtimer_cancel( &invalidate_L1_cache_timer ); 
	}
	// Return data if any
	if( (offset <= 0) && (prf_ready && (cmd != last_cmd)) ){
		// Get the CPU-time compensating for the time spent invalidating L1 cache
		*pOut 			= ELAPSED( t_L1_inv_start , t_L1_inv_end );
		*pNvcsw 		= (d_nvcsw & 0xFF);
		*pNivcsw 		= (d_nivcsw & 0xFF);		
		reset_cntrs();
	}
	else { 
		*pOut 			= 0;
		*pNvcsw 		= 0;
		*pNivcsw 		= 0;		
	}
	// Save cmd for next time
	last_cmd = cmd;
	// Return elapsed time while invalidating L1 cache and the number of volontary and involontary constext switches if we have a profile otherwise 0
	return 	(offset > 0) ? 0 : (sizeof( u64 ) + (2 * sizeof( unsigned char )));;
}

// This funtion is called when the /proc/os_ctrl file is written by user (make sure only 1 user at the time)
int
on_written_handler( \
				struct file 				*file, \
				const char 					*buffer, \
				unsigned long 				count, \
				void 						*data \
			){
	int  					n;
	u64 					invalidate_L1_cache_delay_ns = 0;
	struct pid 				*pApp_id;
	struct task_struct 		*pTask = NULL;
	char 					*pData = os_ctrl_input_buffer, err = 1;
	// Reset the profile ready flag by default
	prf_ready 									= 0;
	// Parse data depending on byte count (NB: Memory is fragmented)
	if( copy_from_user(os_ctrl_input_buffer,buffer,count) == 0 ){
		switch( count ){
			// Stop command length
			case OS_CTRL_STOP_CMD_LENGTH:
				// Get the pid to profile
				p_id								= *((pid_t *)pData);
				// Go on decoding user request: Increment to cmd position
				pData 								+= sizeof( pid_t );
				// Get cmd	
				cmd 								= *((unsigned char *)pData);
				// Rest error flag 
				err 								= 0;
				break;
			// Start command length
			case OS_CTRL_START_CMD_LENGTH:
				// Get the pid to profile
				p_id								= *((pid_t *)pData);
				// Go on decoding user request: Increment to cmd position
				pData 								+= sizeof( pid_t );
				// Get cmd	
				cmd 								= *((unsigned char *)pData);
				// Increment to data position
				pData 								+= sizeof( unsigned char );
				// Get the invalidate L1 cache interrupt delay value and convert from us to ns (0 = no application interrupt)
				invalidate_L1_cache_delay_ns	 	= *((u64 *)pData) * 1000;
				invalidate_L1_cache_delay_ns 		= (invalidate_L1_cache_delay_ns > 999999999L) ? 999999999L : invalidate_L1_cache_delay_ns;
				// Rest error flag 
				err 								= 0;
				break;
			// Garbage data
			default:
				printk( KERN_ALERT "%s ERROR: An error has occurred while parsing input data\n" , MODULE_NAME ); 
				reset_cntrs();
				break;
		}
	}
	// Get the application task pointer from the scheduler (if the application is currently running then no need to look into the stack - spare some cpu cycles) 
	if( !err && (current->pid == p_id) ) 	{ pTask = current; }
	// If application task is not current task the fetch it from the stack (not likely but just in case)
	else if( !err && (!(pApp_id=find_vpid((int)p_id)) || !(pTask=pid_task(pApp_id,PIDTYPE_PID))) ){ 
		printk( KERN_ALERT "%s ERROR: [PID %u] Unable to find task in scheduler\n" , MODULE_NAME , p_id ); 
		// Always enable IRQs
		IRQ_ENABLE();
		reset_cntrs();
		// Set error flag
		err = 1; 
	}
	// Cancel L1 invalidate cache timer
	if( !err && (invalidate_L1_cache_timer.state != HRTIMER_STATE_INACTIVE) ){ 
		hrtimer_cancel( &invalidate_L1_cache_timer ); 
	}
	// Handle command
	if( !err ){
		// Handle command
		switch( cmd ){
			case OS_CTRL_TASK_CMD_DISABLE_OS:
				// Invalidate L1 instruction and data caches
				invalidate_L1_instruction_cache();
				flush_and_invalidate_L1_data_cache();
				// Disable parts of the OS
				set_os( pTask , OS_CTRL_OS_DISABLE );
				// Get current nr of context switches for the task to profile (All cpu-time has been accounted by the scheduler since the task is in the background at the moment)
				d_nvcsw	 						= pTask->nvcsw;
				d_nivcsw		 				= pTask->nivcsw;
				// Reset L1 cache invalidation times
				t_L1_inv_start 					= t_zero;
				t_L1_inv_end					= t_zero;
				// Start L1 cache invalidation timer if needed
				if( invalidate_L1_cache_delay_ns > 0 ){  
					invalidate_L1_cache_timer_int_delay	= ktime_set( 0 , invalidate_L1_cache_delay_ns );
					hrtimer_init( &invalidate_L1_cache_timer , CLOCK_MONOTONIC, HRTIMER_MODE_REL );
					invalidate_L1_cache_timer.function 	= &invalidate_L1_cache_interrupt_handler;
					hrtimer_start( &invalidate_L1_cache_timer , invalidate_L1_cache_timer_int_delay , HRTIMER_MODE_REL );
				} 
				break;
			case OS_CTRL_TASK_CMD_ENABLE_OS:
				// Get current nr of context switches for the task to profile (All cpu-time has been accounted by the scheduler since the task is in the background at the moment)
				d_nvcsw	 						= ((pTask->nvcsw > d_nvcsw) ? (pTask->nvcsw-d_nvcsw) : 0);
				d_nivcsw		 				= ((pTask->nivcsw > d_nivcsw) ? (pTask->nivcsw-d_nivcsw) : 0);
				// Set the profile ready flag
				prf_ready 						= 1;
				// Enable OS
				set_os( pTask , OS_CTRL_OS_ENABLE );
				break;
			// Default: Ignore cmd
			default:
				printk( KERN_ALERT "%s ERROR: [PID %u] Unknown CMD 0x%02X\n" , MODULE_NAME , p_id , cmd ); 
				break;
		}
	}
	return (err) ? -ENOMEM : count;
}


// Init module (called @ insmod)
int 
init_module( void ){	
	int ret = 0;
	printk( KERN_INFO "%s: Loading module\n" , MODULE_NAME );
	reset_cntrs();
	set_normalized_timespec( &t_zero , 0 , 0 );
	// Always reset scheduler to normal behaviour
	set_pid_force_run( 0 );	
	// Create /proc/os_ctrl file
	if( !(os_ctrl_proc_file=create_proc_entry(MODULE_NAME,0666,NULL)) ){
		remove_proc_entry( "profiler" , NULL );
		printk( KERN_ALERT "%s ERROR: Could not create /proc/%s\n" , 	MODULE_NAME , MODULE_NAME );
		ret = -ENOMEM;
	}
	else {
		t_L1_inv_start 					= t_zero;
		t_L1_inv_end 					= t_zero;
		os_ctrl_proc_file->mode 	 	= 0666;
		os_ctrl_proc_file->uid 	 		= 0;
		os_ctrl_proc_file->gid 	 		= 0;
		os_ctrl_proc_file->size 	 	= PROCFS_MAX_SIZE;
		os_ctrl_proc_file->read_proc 	= on_read_handler; 		
		os_ctrl_proc_file->write_proc 	= on_written_handler; 	
		printk( KERN_INFO "%s: Created /proc/%s (rw)\n" , 	MODULE_NAME , MODULE_NAME );		
		printk( KERN_INFO "%s: Scheduler reset timeout set to %lu ns\n" , MODULE_NAME , SCHED_RESET_DELAY_NS );
		printk( KERN_INFO "%s: Application timeout set to %d s\n" , MODULE_NAME , KILL_APPLICATION_DELAY_S );
		memset( &kill_app_signal , 0 , sizeof(struct siginfo) );
		kill_app_signal.si_signo		= SIGKILL;
		kill_app_signal.si_code			= SI_KERNEL;
		kill_app_signal.si_int			= 0;	
	}	
	// A non 0 return means init_module failed; module can't be loaded. 
	return ret;
}

// Clean up module (called @ rmmod)
void 
cleanup_module( void ){
	int n;
	// Always reset scheduler to normal behaviour
	set_pid_force_run( 0 );	
	// Enable IRQs if needed
	IRQ_ENABLE();
	// Cancel scheduler reset timer
	if( sched_reset_timer.state != HRTIMER_STATE_INACTIVE ){ 
		printk( KERN_INFO "%s: Stopping scheduler reset timeout timer\n", 		MODULE_NAME );
		hrtimer_cancel( &sched_reset_timer ); 
	}
	// Cancel application timeout timer (if it is a start/stop cmd only)
	if( kill_application_timer.state != HRTIMER_STATE_INACTIVE ){ 
		printk( KERN_INFO "%s: Stopping application timeout timer\n", 		MODULE_NAME );
		hrtimer_cancel( &kill_application_timer ); 
	}
	// Cancel L1 invalidate cache timer (if it is a start/stop cmd only)
	if( invalidate_L1_cache_timer.state != HRTIMER_STATE_INACTIVE ){ 
		printk( KERN_INFO "%s: Stopping L1 invalidation cache timer\n", 	MODULE_NAME );
		hrtimer_cancel( &invalidate_L1_cache_timer ); 
	}
	// Remove proc files
	printk( KERN_INFO "%s: Removing /proc/%s\n", 	MODULE_NAME , MODULE_NAME );
	remove_proc_entry( MODULE_NAME , NULL );
	printk( KERN_INFO "%s: Unloaded module\n" , 	MODULE_NAME );
}
