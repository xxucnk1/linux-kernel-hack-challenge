/* Item: test_extensions.c

   Description: Defines the common scripting mechanisms.

   Revision: %PID%
   
   Revision History:
   IP #          Date       Programmer     Remarks
   ------------- ---------- -------------- --------------------------

   */

#include <sys/time.h>
#include "test_extensions.h"
#include <stdio.h>
#include <alloca.h>


void INVALIDATE_L1(void) {
#ifdef _BREADBOARD_EXECUTION
  FILE *file;
  if (file = fopen("/proc/Invalidate_L1","w")) {
    /* Writing to file Invalidates L1 cache */
    fprintf(file,"%s","Write to a file ");
    fclose(file);
  } else {
    printf("*** ERROR: L1 cache invalidation is not available on this platform\n");
  }
#endif
}

