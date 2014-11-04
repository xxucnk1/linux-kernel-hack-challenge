/*****************************************************************************/
/*    Filename: test_time_a60hz.c
 *    Generated on: 29-Jan-2013 09:57:34
 *    Generated from: time_a60hz.c
 */
/*****************************************************************************/
/* Environment Definition                                                    */
/*****************************************************************************/


/* Include files from software under test */
#include <stdio.h>
#include <test_extensions.h>

/* Global Functions */
extern void time_A60Hz_Initialize(void);
extern void time_A60Hz_Execute(void);

/* Global data */
/* None */

/* Expected variables for global data */
/* None */

/* This function initialises global data to default values. This function       */
/* is called by every test case so must not contain test case specific settings */
static void initialise_global_data(){
    /* No global data */
}

/* This function copies the global data settings into expected variables for */
/* use in check_global_data(). It is called by every test case so must not   */
/* contain test case specific settings.                                      */
static void initialise_expected_global_data(){
    /* No global data */
}

/* This function checks global data against the expected values. */
static void check_global_data(){
    /* No global data */
}

/* Prototypes for test functions */
void run_tests(void);
void test_check(int);


/*****************************************************************************/
/* Program Entry Point                                                       */
/*****************************************************************************/
int main()
{
    run_tests();

    return 0;
}

/*****************************************************************************/
/* Test Control                                                              */
/*****************************************************************************/
/* run_tests() contains calls to the individual test cases, you can turn test*/
/* cases off by adding comments*/
void run_tests(void)
{
    test_check(1);

}

/*****************************************************************************/
/* Test Cases                                                                */
/*****************************************************************************/

void test_check(int doIt){

  if (doIt) {
      initialise_global_data();
    /* Set expected values for global data checks */
    initialise_expected_global_data();


    time_A60Hz_Initialize();

    for(int i=0;i<10;i++){
      INVALIDATE_L1();            // Invalidate L1 cache
      time_A60Hz_Execute();
    }

  }

}


/*****************************************************************************/
/* End of test script                                                        */
/*****************************************************************************/
