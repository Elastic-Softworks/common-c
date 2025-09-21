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
#include  <string.h>

/*
	==================================
             --- GLOBALS ---
	==================================
*/

static commc_error_context_t error_context = {0};

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

         commc_log()
	       ---
	       unified logging system that outputs messages with
	       consistent C-FORM formatting. uses 'OUTPUT: ...' 
	       prefix pattern for educational clarity and to
	       match established project output standards.

*/

void commc_log(commc_log_level_t level, const char* message) {

  const char* level_prefixes[] = {
    "DEBUG", "INFO", "WARN", "ERROR"
  };

  if  (!message) {

    return;

  }

  printf("OUTPUT: %s %s\n", level_prefixes[level], message);

}

/*

         commc_log_debug()
	       ---
	       debug-level logging wrapper. in production builds,
	       debug messages may be disabled for performance.
	       useful for detailed algorithm tracing and state inspection.

*/

void commc_log_debug(const char* message) {

#ifdef DEBUG

  commc_log(COMMC_LOG_DEBUG, message);

#else

  (void)message; /* suppress unused parameter warning */
  
#endif

}

/*

         commc_log_info()
	       ---
	       informational logging for normal operational messages.
	       used to provide user feedback about successful operations,
	       progress updates, and general system status.

*/

void commc_log_info(const char* message) {

  commc_log(COMMC_LOG_INFO, message);

}

/*

         commc_log_warn()
	       ---
	       warning-level logging for non-fatal issues.
	       indicates conditions that should be addressed
	       but don't prevent continued execution.

*/

void commc_log_warn(const char* message) {

  commc_log(COMMC_LOG_WARN, message);

}

/*

         commc_log_error()
	       ---
	       error-level logging for serious problems.
	       indicates conditions that may cause system
	       instability or incorrect behavior.

*/

void commc_log_error(const char* message) {

  commc_log(COMMC_LOG_ERROR, message);

}

/*

         commc_error_set_context()
	       ---
	       captures comprehensive error context for detailed
	       reporting. this educational system helps developers
	       understand not just what went wrong, but exactly
	       where and why it happened in the codebase.

*/

void commc_error_set_context(commc_error_t error, const char* file, int line, 
                             const char* function, const char* custom_message) {

  error_context.error_code    = error;
  error_context.file_name     = file;
  error_context.line_number   = line;
  error_context.function_name = function;

  if  (custom_message) {

    strncpy(error_context.custom_message, custom_message, 255);
    error_context.custom_message[255] = '\0'; /* ensure null termination */

  } else {

    error_context.custom_message[0] = '\0';

  }

}

/*

         commc_error_get_detailed_message()
	       ---
	       generates comprehensive error message combining
	       error code, location information, and custom details.
	       designed for educational debugging to help new
	       programmers understand error context completely.

*/

const char* commc_error_get_detailed_message(void) {

  static char detailed_message[512];

  sprintf(detailed_message,
          "ERROR [%d]: %s\n"
          "  FILE: %s\n"
          "  LINE: %d\n" 
          "  FUNCTION: %s\n"
          "  DETAILS: %s",
          error_context.error_code,
          commc_error_message(error_context.error_code),
          error_context.file_name ? error_context.file_name : "unknown",
          error_context.line_number,
          error_context.function_name ? error_context.function_name : "unknown", 
          error_context.custom_message[0] ? error_context.custom_message : "none");

  return detailed_message;

}

/*
	==================================
             --- EOF ---
	==================================
*/
