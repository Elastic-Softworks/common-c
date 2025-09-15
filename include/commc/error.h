/*
   ===================================
   C O M M O N - C
   ERROR HANDLING MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

              --- ERROR MODULE ---

    This module provides core error handling utilities
    for the COMMON-C library suite. It includes simple
    assertions, error codes, and basic error reporting
    to help developers catch issues early in C89.

    There are no dependencies on external libraries;
    everything is self-contained and minimal.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_ERROR_H
#define COMMC_ERROR_H

#include <stdio.h>

/*
	==================================
             --- DEFINES ---
	==================================
*/

typedef enum {

  COMMC_SUCCESS,        /* 0: NO ERROR */
  COMMC_FAILURE,        /* 1: GENERAL FAILURE */
  COMMC_MEMORY_ERROR,   /* 2: MEMORY ALLOCATION FAILURE */
  COMMC_ARGUMENT_ERROR, /* 3: INVALID ARGUMENT */
  COMMC_IO_ERROR,       /* 4: FILE OR I/O ERROR */
  COMMC_SYSTEM_ERROR    /* 5: SYSTEM-SPECIFIC ERROR */

} commc_error_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

          commc_error_message()
          ---
          retrieve a human-readable string for
          an error code. useful for logging or
          user feedback.

*/

const char* commc_error_message(commc_error_t error);

/*

           commc_assert()
           ---
           simple integer-based assertion that
           prints an error message and returns
           a failure code if the condition fails.
           unlike the standard assert, this is
           configurable and doesn't exit the program.

*/

commc_error_t commc_assert(int condition, const char* message);

/*

           commc_report_error()
           ---
           print an error message to stderr with
           optional file and line info for debugging.

*/

void commc_report_error(commc_error_t error, const char* file, int line);

/* macro wrapper for assertions with file/line info. */

#define COMMC_ASSERT(condition, msg) \
  if (!commc_assert((condition), (msg))) { \
    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__); \
  }

#endif /* COMMC_ERROR_H */

/*
	==================================
             --- EOF ---
	==================================
*/
