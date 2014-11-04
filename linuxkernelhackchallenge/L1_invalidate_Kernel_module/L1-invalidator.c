/**  
 *  L1-invalidator.c - Flush and invalidate L1 cache
 *
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <linux/slab.h>         /* Needed for kmalloc and kfree */

#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */


#define PROCFS_NAME  "Invalidate_L1"
#define SCRATCH_SIZE 49152      /* 12 * 128 * 32 */

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *L1_Proc_File;


/**
 * These functions do the actual work
 *
 */
void invalidate_L1_instruction_cache(void)
{
  asm volatile("mfspr 3, 1011;      /* Read L1CSR1 to R3              */\
                ori   4, 3, 0x0002; /* R4 as R3 but with ICFI bit set */\
                mtspr 1011, 4;      /* Invalidate by writing ICFI bit */\
                mtspr 1011, 3;      /* Restore L1CSR1                 */\
                isync;"
                : /* no output */
                : /* no input */
                : "3", "4", "memory" /* GPR3 and GPR4 clobbered */);
  return;
}


void flush_and_invalidate_L1_data_cache(void)
{
  void* scratchref = NULL;
  void* scratchend = NULL;

  scratchref = kmalloc(SCRATCH_SIZE, GFP_KERNEL);
  if (scratchref) {
    // printk(KERN_INFO "MEM allocated\n");
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
  else {
    printk(KERN_ALERT "Error: MEM not allocated!\n");
  }
  return;
}


/**
 * This function is called when the /proc file is read
 *
 */
int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
  // This is not the intended usage, but nothing to panic about
  printk(KERN_ALERT "Bad usage: /proc/%s read\n", PROCFS_NAME);
  return 0;
}


/**
 * This function is called when the /proc file is written
 *
 */
int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
  // Do the work
  invalidate_L1_instruction_cache();
  flush_and_invalidate_L1_data_cache();
  return count;
}


/**
 * These are the setup and teardown functions of the module
 *
 */
static int __init L1_inv_init(void)
{
  printk(KERN_INFO "Starting L1 cache invalidating kernel module\n");

  /* create the /proc file */
  L1_Proc_File = create_proc_entry(PROCFS_NAME, 0222, NULL);
	
  if (L1_Proc_File == NULL) {
    printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
    return -ENOMEM;
  }
	
  L1_Proc_File->read_proc  = procfile_read;
  L1_Proc_File->write_proc = procfile_write;
  L1_Proc_File->uid        = 0;
  L1_Proc_File->gid        = 0;
  L1_Proc_File->size       = 37;

  printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);	
  return 0;	/* everything is ok */
	
}


static void __exit L1_inv_exit(void)
{
  printk(KERN_INFO "Unloaded L1 cache invalidating kernel module\n");
  // Remove the /proc file
  remove_proc_entry(PROCFS_NAME, L1_Proc_File->parent);
  return;
}


module_init(L1_inv_init);
module_exit(L1_inv_exit);
