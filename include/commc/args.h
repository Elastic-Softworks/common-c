/*
   ===================================
   C O M M O N - C
   COMMAND-LINE ARGUMENT PARSING MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- ARGS MODULE ---

    this module provides a simple, C89-compliant way
    to parse command-line arguments. it supports flags,
    options with values, and positional arguments.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_ARGS_H
#define COMMC_ARGS_H

#include <stddef.h>       /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

typedef enum {

  COMMC_ARG_FLAG,        /* boolean flag (e.g., -v, --verbose) */
  COMMC_ARG_OPTION,      /* option with a value (e.g., -o output.txt, --file=input.txt) */
  COMMC_ARG_POSITIONAL   /* positional argument (e.g., myapp file1 file2) */

} commc_arg_type_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* opaque argument parser context. */

typedef struct commc_args_parser_t commc_args_parser_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_args_parser_create()
	       ---
	       creates a new argument parser instance.

*/

commc_args_parser_t* commc_args_parser_create(void);

/*

         commc_args_parser_destroy()
	       ---
	       frees all memory associated with the parser.

*/

void commc_args_parser_destroy(commc_args_parser_t* parser);

/*

         commc_args_add_flag()
	       ---
	       adds a boolean flag argument.

*/

int commc_args_add_flag(commc_args_parser_t* parser, const char* short_name, const char* long_name, const char* description);

/*

         commc_args_add_option()
	       ---
	       adds an option argument that expects a value.

*/

int commc_args_add_option(commc_args_parser_t* parser, const char* short_name, const char* long_name, const char* description, const char* default_value);

/*

         commc_args_add_positional()
	       ---
	       adds a positional argument.

*/

int commc_args_add_positional(commc_args_parser_t* parser, const char* name, const char* description);

/*

         commc_args_parse()
	       ---
	       parses the command-line arguments.
	       returns 1 on success, 0 on failure.

*/

int commc_args_parse(commc_args_parser_t* parser, int argc, char* argv[]);

/*

         commc_args_get_flag()
	       ---
	       retrieves the value of a flag.
	       returns 1 if present, 0 otherwise.

*/

int commc_args_get_flag(commc_args_parser_t* parser, const char* name);

/*

         commc_args_get_option()
	       ---
	       retrieves the value of an option.
	       returns the value string, or default if not found.

*/

const char* commc_args_get_option(commc_args_parser_t* parser, const char* name);

/*

         commc_args_get_positional()
	       ---
	       retrieves a positional argument by its index.
	       returns the value string, or NULL if not found.

*/

const char* commc_args_get_positional(commc_args_parser_t* parser, size_t index);

/*

         commc_args_print_help()
	       ---
	       prints the help message for the parser.

*/

void commc_args_print_help(commc_args_parser_t* parser, const char* program_name);

#endif /* COMMC_ARGS_H */

/*
	==================================
             --- EOF ---
	==================================
*/
