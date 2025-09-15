/*
   ===================================
   C O M M O N - C
   ERROR IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- ERROR MODULE ---

    implementation of error handling functions.
    see include/commc/error.h for prototypes.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  "commc/error.h"

/*
	==================================
             --- GLOBALS ---
	==================================
*/

static const char* error_messages[] = {

  "no error",                  /* COMMC_SUCCESS */
  "general failure",           /* COMMC_FAILURE */
  "memory allocation failure", /* COMMC_MEMORY_ERROR */
  "invalid argument",          /* COMMC_ARGUMENT_ERROR */
  "file or I/O error",         /* COMMC_IO_ERROR */
  "system-specific error"      /* COMMC_SYSTEM_ERROR */

};

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_error_message()
	        ---
	        returns a string description of the error code.
	        useful for user feedback without crashing the program.

*/

const char* commc_error_message(commc_error_t error) {

  if  (error < COMMC_SUCCESS || error >= sizeof(error_messages) / sizeof(const char*)) {

    return "unknown error";

  } else {

    return error_messages[error];

  }

}

/*

         commc_assert()
	       ---
	       performs a simple assertion check. If the condition
	       is false, it prints the message and returns failure.
	       this doesn't exit like standard assert, allowing
	       graceful error handling in embedded or game contexts.

*/

commc_error_t commc_assert(int condition, const char* message) {

  if  (!condition) {

    fprintf(stderr, "assertion failed: %s\n", message);
    return COMMC_FAILURE;

  } else {

    return COMMC_SUCCESS;

  }

}

/*

         commc_report_error()
	       ---
	       reports an error to stderr with optional file and
	       line information for easier debugging during development.

*/

void commc_report_error(commc_error_t error, const char* file, int line) {

  fprintf(stderr, "error [%d]: %s at %s:%d\n", error, commc_error_message(error), file, line);

}

/*
	==================================
             --- EOF ---
	==================================
*/
