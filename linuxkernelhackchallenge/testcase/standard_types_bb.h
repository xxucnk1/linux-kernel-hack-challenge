// ******************************************************************
// Item: standard_types_bb.h
//
// Description: This files defines the basic types that shall be used by
//              all Applications instead of the native C types.
//
// Revision: %PID%
//
// Revision History:
// IP #          Date       Programmer     Remarks
// ------------- ---------- -------------- --------------------------
//
// ******************************************************************

#ifndef STANDARD_TYPES_H
#define STANDARD_TYPES_H

// ******************************************************************
// INCLUDE
// ******************************************************************

// ******************************************************************
// DEFINES
// ******************************************************************

#undef FALSE
#undef TRUE

#define FALSE (0)
#define TRUE  (1)

// ******************************************************************
// TYPES
// ******************************************************************

typedef signed   long long Integer64_Type ;
typedef unsigned long long Unsigned64_Type;

typedef signed   long      Integer32_Type ;
typedef unsigned long      Unsigned32_Type;

typedef signed   short     Integer16_Type;
typedef unsigned short     Unsigned16_Type;

typedef signed   char      Integer8_Type;
typedef unsigned char      Unsigned8_Type;

typedef signed char 	   Boolean_Type;

typedef signed char        Character_Type;

typedef float              Real32_Type;
typedef double             Real64_Type;

typedef Unsigned32_Type    Address_Type;

// ******************************************************************
// CONSTANTS
// ******************************************************************

// N/A

// ******************************************************************
// VARIABLES
// ******************************************************************

// N/A

// ******************************************************************
// FUNCTIONS
// ******************************************************************

// N/A


#endif /* STANDARD_TYPES_H */
