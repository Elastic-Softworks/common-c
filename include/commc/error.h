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

#ifndef   COMMC_ERROR_H
#define   COMMC_ERROR_H

#include  <stdio.h>

/*
	==================================
             --- DEFINES ---
	==================================
*/

typedef enum {

  COMMC_SUCCESS,                   /* 0: NO ERROR */
  COMMC_FAILURE,                   /* 1: GENERAL FAILURE */
  COMMC_MEMORY_ERROR,              /* 2: MEMORY ALLOCATION FAILURE */
  COMMC_ARGUMENT_ERROR,            /* 3: INVALID ARGUMENT */
  COMMC_IO_ERROR,                  /* 4: FILE OR I/O ERROR */
  COMMC_SYSTEM_ERROR,              /* 5: SYSTEM-SPECIFIC ERROR */
  COMMC_NOT_IMPLEMENTED_ERROR,     /* 6: FEATURE NOT YET IMPLEMENTED */
  COMMC_FORMAT_ERROR,              /* 7: INVALID DATA FORMAT */
  COMMC_VERSION_ERROR,             /* 8: INCOMPATIBLE VERSION */
  COMMC_ERROR_INVALID_ARGUMENT,    /* 9: INVALID FUNCTION ARGUMENT */
  COMMC_ERROR_INVALID_STATE,       /* 10: INVALID OBJECT STATE */
  COMMC_ERROR_BUFFER_TOO_SMALL,    /* 11: OUTPUT BUFFER TOO SMALL */
  COMMC_ERROR_INVALID_DATA,        /* 12: INVALID INPUT DATA */
  COMMC_ERROR_TIMEOUT,             /* 13: OPERATION TIMED OUT */
  COMMC_ERROR_WOULD_BLOCK,         /* 14: OPERATION WOULD BLOCK */
  COMMC_ERROR_CONNECTION_CLOSED    /* 15: CONNECTION WAS CLOSED */

} commc_error_t;

/*
	==================================
             --- LOG LEVELS ---
	==================================
*/

typedef enum {

  COMMC_LOG_DEBUG,        /* 0: DETAILED DEBUG INFORMATION */
  COMMC_LOG_INFO,         /* 1: GENERAL INFORMATION MESSAGES */  
  COMMC_LOG_WARN,         /* 2: WARNING CONDITIONS */
  COMMC_LOG_ERROR         /* 3: ERROR CONDITIONS */

} commc_log_level_t;

/*
	==================================
             --- ERROR CONTEXT ---
	==================================
*/

typedef struct {

  commc_error_t   error_code;           /* THE ERROR TYPE */
  const char*     file_name;            /* SOURCE FILE WHERE ERROR OCCURRED */
  int             line_number;          /* LINE NUMBER IN SOURCE FILE */
  const char*     function_name;        /* FUNCTION WHERE ERROR OCCURRED */
  char            custom_message[256];  /* ADDITIONAL ERROR DETAILS */

} commc_error_context_t;

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

/*

           commc_log()
           ---
           unified logging function with severity levels.
           follows C-FORM 'OUTPUT: ...' formatting standard
           for educational clarity and consistent messaging.

*/

void commc_log(commc_log_level_t level, const char* message);

/*

           commc_log_debug()
           ---
           convenience wrapper for debug-level messages.
           only outputs in debug builds for performance.

*/

void commc_log_debug(const char* message);

/*

           commc_log_info()
           ---
           convenience wrapper for informational messages.
           used for normal operational feedback.

*/

void commc_log_info(const char* message);

/*

           commc_log_warn()
           ---
           convenience wrapper for warning messages.
           indicates potential issues that don't stop execution.

*/

void commc_log_warn(const char* message);

/*

           commc_log_error()
           ---
           convenience wrapper for error messages.
           indicates serious problems requiring attention.

*/

void commc_log_error(const char* message);

/*

           commc_error_set_context()
           ---
           sets detailed error context information for enhanced
           debugging. stores file, line, function, and custom 
           message for comprehensive error reporting.

*/

void commc_error_set_context(commc_error_t error, const char* file, int line, 
                             const char* function, const char* custom_message);

/*

           commc_error_get_detailed_message()
           ---
           retrieves comprehensive error message with full context.
           returns formatted string with file, line, function, and
           custom message information for educational debugging.

*/

const char* commc_error_get_detailed_message(void);

/* macro wrapper for assertions with file/line info. */

#define COMMC_ASSERT(condition, msg) \
  if (commc_assert((condition), (msg)) != COMMC_SUCCESS) { \
    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__); \
  }

#endif /* COMMC_ERROR_H */

/*
	==================================
             --- EOF ---
	==================================
*/
