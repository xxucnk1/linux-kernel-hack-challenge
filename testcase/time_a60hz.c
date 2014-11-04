// ******************************************************************
// Item: time_a60hz.c
//
// Description: Test application
//
// Revision: %PID%
//
// Revision History:
// IP #          Date       Programmer     Remarks
// ------------- ---------- -------------- --------------------------
//
// ******************************************************************

// ******************************************************************
// INCLUDE
// ******************************************************************

#include <stdio.h>
#include "libprf.h"
#include "standard_types_bb.h"
#include <math.h>
#include <test_extensions.h>

// ******************************************************************
// DEFINES
// ******************************************************************


// ******************************************************************
// TYPES
// ******************************************************************

#include "time_types.c"  /* Ouch! */


// ******************************************************************
// PUBLIC CONSTANTS
// ******************************************************************

// ******************************************************************
// LOCAL CONSTANTS
// ******************************************************************

// ******************************************************************
// PUBLIC VARIABLES
// ******************************************************************

// ******************************************************************
// LOCAL VARIABLES
// ******************************************************************


// ******************************************************************
// FUNCTIONS
// ******************************************************************

/* Declare exported functions (we have no h-file for this module) */

void time_A60Hz_Initialize(void);
void time_A60Hz_Execute(void);

// ******************************************************************
// Description: Application initialize function
// ******************************************************************
void time_A60Hz_Initialize(void)
{
  Time_Init();
}

// ******************************************************************
// Description: Application execute function
// ******************************************************************
void time_A60Hz_Execute(void)
{
  Integer32_Type i = 0;
  
  long long int Time_Spent11 = 0;

  long long int Time_Spent21 = 0;
    
  long long int Time_Spent31 = 0;
  
  long long int Time_Spent41 = 0;
 
  long long int Time_Spent51 = 0;

  //Profiling 
  long long int Tint = 0;
  long long int Ttimeout = 100000; //us
  long long int Texe = 2;
  long long int Texe_First = 0;
  int Texe_Old = -1;
  int Diff = 0;
  int ret = 0;
  
// Profiling
  printf("Measuring...\n");
  while (Texe > Tint*1000 || Tint < 350 )
  {
    // Execute Application
    // Invalidate L1 before execution of application
    INVALIDATE_L1();
    START_PROFILING(Tint, Ttimeout);  // Tint and Ttimeout in us
    for (i = 0; i < 1; i++){
      Time_TypeP3(); // Little cache usage
      Time_TypeP4(); // Some L1 data cache usage
      Time_TypeP5(); // Extensive L1 data cache usage
    }
    ret = (int)STOP_PROFILING(&Texe);  // Texe in ns
    if (Texe_First == 0) {
      Texe_First = Texe; // First sample without interrupts. No execution can be faster than this
    }
    Diff = abs(Texe - Texe_Old); // nanoseconds
    // Measurement reported OK. => ret = 0
    // Execution time must be larger than first reported without interrupts
    // The increase in time shall be small enough, 1us extra execution can not 
    // have an execution time of more than 1us due to L1 cache effects.
    printf("Tint = %lli, Texe = %lli, ret= %i\n", Tint, Texe/1000, ret);
    Texe_Old = Texe;
    Tint = Tint + 1;
  }   
 

} /* time_A60Hz_Execute */
