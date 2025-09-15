/*
   ===================================
   C O M M O N - C
   COMMAND-LINE ARGUMENT PARSING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- ARGS MODULE ---

    implementation of the command-line argument parser.
    see include/commc/args.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/args.h"
#include "commc/error.h"     /* for error handling */
#include "commc/string.h"    /* for string utilities */
#include <stdlib.h>          /* for malloc, free */
#include <string.h>          /* for strcmp, strdup, strlen */
#include <stdio.h>           /* for printf, fprintf */

/*
	==================================
             --- DEFINES ---
	==================================
*/

#define MAX_ARGS_COUNT 64    /* maximum number of arguments supported */
#define MAX_NAME_LEN   32    /* maximum length for short/long names */
#define MAX_DESC_LEN   128   /* maximum length for description */
#define MAX_VALUE_LEN  256   /* maximum length for option values */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal representation of a registered argument. */

typedef struct {

  commc_arg_type_t type;

  char  short_name[MAX_NAME_LEN];
  char  long_name[MAX_NAME_LEN];
  char  description[MAX_DESC_LEN];
  char  default_value[MAX_VALUE_LEN];   /* for options */
  char  parsed_value[MAX_VALUE_LEN];    /* actual parsed value */
  int   is_set;                         /* for flags and options */

} commc_arg_def_t;

/* argument parser context. */

struct commc_args_parser_t {

  commc_arg_def_t args[MAX_ARGS_COUNT];
  size_t          num_args;
  char**          positional_values;
  size_t          num_positional_parsed;
  size_t          max_positional_defs;

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         find_arg_def()
	       ---
	       finds an argument definition by its short or long name.

*/

static commc_arg_def_t* find_arg_def(commc_args_parser_t* parser, const char* name) {

  size_t i;

  for  (i = 0; i < parser->num_args; i++) {

    if  ((parser->args[i].short_name[0] != '\0' && strcmp(parser->args[i].short_name, name) == 0) ||
         (parser->args[i].long_name[0]  != '\0' && strcmp(parser->args[i].long_name,  name) == 0)) {

      return &parser->args[i];

    }

  }

  return NULL;

}

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_args_parser_create()
	       ---
	       allocates and initializes a new argument parser.

*/

commc_args_parser_t* commc_args_parser_create(void) {

  commc_args_parser_t* parser;
  
  parser = (commc_args_parser_t*)malloc(sizeof(commc_args_parser_t));

  if  (!parser) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  memset(parser, 0, sizeof(commc_args_parser_t));

  parser->num_args            = 0;
  parser->positional_values   = NULL;
  parser->num_positional_parsed = 0;
  parser->max_positional_defs = 0;

  return parser;

}

/*

         commc_args_parser_destroy()
	       ---
	       frees all memory associated with the parser.

*/

void commc_args_parser_destroy(commc_args_parser_t* parser) {

  size_t i;

  if  (!parser) {

    return;

  }

  if  (parser->positional_values) {

    for  (i = 0; i < parser->num_positional_parsed; i++) {

      free(parser->positional_values[i]);

    }

    free(parser->positional_values);

  }

  free(parser);

}

/*

         commc_args_add_flag()
	       ---
	       registers a new boolean flag.

*/

int commc_args_add_flag(commc_args_parser_t* parser, const char* short_name, const char* long_name, const char* description) {

  commc_arg_def_t* arg_def;

  if  (!parser || parser->num_args >= MAX_ARGS_COUNT) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return 0;

  }

  arg_def = &parser->args[parser->num_args];
  memset(arg_def, 0, sizeof(commc_arg_def_t));

  arg_def->type = COMMC_ARG_FLAG;

  if  (short_name) {

    commc_string_copy(arg_def->short_name, short_name, MAX_NAME_LEN);

  }

  if  (long_name) {

    commc_string_copy(arg_def->long_name, long_name, MAX_NAME_LEN);

  }

  if  (description) {

    commc_string_copy(arg_def->description, description, MAX_DESC_LEN);

  }

  parser->num_args++;

  return 1;

}

/*

         commc_args_add_option()
	       ---
	       registers a new option that expects a value.

*/

int commc_args_add_option(commc_args_parser_t* parser, const char* short_name, const char* long_name, const char* description, const char* default_value) {

  commc_arg_def_t* arg_def;

  if  (!parser || parser->num_args >= MAX_ARGS_COUNT) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return 0;

  }

  arg_def = &parser->args[parser->num_args];
  memset(arg_def, 0, sizeof(commc_arg_def_t));

  arg_def->type = COMMC_ARG_OPTION;

  if  (short_name) {

    commc_string_copy(arg_def->short_name, short_name, MAX_NAME_LEN);

  }

  if  (long_name) {

    commc_string_copy(arg_def->long_name, long_name, MAX_NAME_LEN);

  }

  if  (description) {

    commc_string_copy(arg_def->description, description, MAX_DESC_LEN);

  }

  if  (default_value) {

    commc_string_copy(arg_def->default_value, default_value, MAX_VALUE_LEN);

  }

  parser->num_args++;

  return 1;

}

/*

         commc_args_add_positional()
	       ---
	       registers a new positional argument.

*/

int commc_args_add_positional(commc_args_parser_t* parser, const char* name, const char* description) {

  commc_arg_def_t* arg_def;

  if  (!parser || parser->num_args >= MAX_ARGS_COUNT) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return 0;

  }

  arg_def = &parser->args[parser->num_args];
  memset(arg_def, 0, sizeof(commc_arg_def_t));

  arg_def->type = COMMC_ARG_POSITIONAL;

  if  (name) {

    /* use long_name for positional name */

    commc_string_copy(arg_def->long_name, name, MAX_NAME_LEN);

  }

  if  (description) {

    commc_string_copy(arg_def->description, description, MAX_DESC_LEN);

  }

  parser->num_args++;
  parser->max_positional_defs++;

  return 1;

}

/*

         commc_args_parse()
	       ---
	       parses the command-line arguments provided.

*/

int commc_args_parse(commc_args_parser_t* parser, int argc, char* argv[]) {

  int  i;

  commc_arg_def_t*  arg_def;

  char*   eq_pos;
  char    temp_name[MAX_NAME_LEN];
  char    temp_value[MAX_VALUE_LEN];

  size_t  positional_idx = 0;

  if  (!parser || !argv) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  /* allocate space for positional arguments */

  if  (parser->max_positional_defs > 0) {

    parser->positional_values = (char**)malloc(sizeof(char*) * parser->max_positional_defs);

    if  (!parser->positional_values) {

      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
      return 0;

    }

    memset(parser->positional_values, 0, sizeof(char*) * parser->max_positional_defs);

  }

  for  (i = 1; i < argc; i++) {     /* skip program name */

    if  (argv[i][0] == '-') {       /* flag or option */

      if  (argv[i][1] == '-') {     /* long name */

        eq_pos = strchr(argv[i], '=');

        if  (eq_pos) {              /* option with value: --option=value */

          commc_string_copy(temp_name, argv[i] + 2, (size_t)(eq_pos - (argv[i] + 2) + 1));
          commc_string_copy(temp_value, eq_pos + 1, MAX_VALUE_LEN);

          arg_def = find_arg_def(parser, temp_name);

          if  (arg_def && arg_def->type == COMMC_ARG_OPTION) {

            commc_string_copy(arg_def->parsed_value, temp_value, MAX_VALUE_LEN);
            arg_def->is_set = 1;

          } else {

            commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

            return 0;                /* unknown or invalid argument */

          }

        } else {                     /* flag or option without '=': --flag or --option value */

          arg_def = find_arg_def(parser, argv[i] + 2);

          if  (arg_def) {

            if  (arg_def->type == COMMC_ARG_FLAG) {

              arg_def->is_set = 1;

            } else if  (arg_def->type == COMMC_ARG_OPTION) {

              if  (i + 1 < argc) {    /* check for value */

                commc_string_copy(arg_def->parsed_value, argv[i+1], MAX_VALUE_LEN);
                arg_def->is_set = 1;

                i++;                  /* consume next argument as value */

              } else {

                commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

                return 0;             /* option requires value */

              }

            }

          } else {

            commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

            return 0;                 /* unknown argument */

          }

        }

      } else {                        /* short name: -f or -o value */

        arg_def = find_arg_def(parser, argv[i] + 1);

        if  (arg_def) {

          if  (arg_def->type == COMMC_ARG_FLAG) {

            arg_def->is_set = 1;

          } else if  (arg_def->type == COMMC_ARG_OPTION) {

            if  (i + 1 < argc) {       /* check for value */

              commc_string_copy(arg_def->parsed_value, argv[i+1], MAX_VALUE_LEN);
              arg_def->is_set = 1;

              i++;                     /* consume next argument as value */

            } else {

              commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

              return 0;                /* option requires value */

            }

          }

        } else {

          commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

          return 0;                    /* unknown argument */

        }

      }

    } else {                          /* positional argument */

      if  (positional_idx < parser->max_positional_defs) {

        parser->positional_values[positional_idx] = strdup(argv[i]);

        if  (!parser->positional_values[positional_idx]) {

          commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
          return 0;

        }

        parser->num_positional_parsed++;
        positional_idx++;

      } else {

        commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

        return 0;                   /* too many positional arguments */

      }

    }

  }

  return 1;

}

/*

         commc_args_get_flag()
	       ---
	       retrieves the value of a flag.

*/

int commc_args_get_flag(commc_args_parser_t* parser, const char* name) {

  commc_arg_def_t* arg_def;

  if  (!parser || !name) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  arg_def = find_arg_def(parser, name);

  if  (arg_def && arg_def->type == COMMC_ARG_FLAG) {

    return arg_def->is_set;

  }

  return 0;

}

/*

         commc_args_get_option()
	       ---
	       retrieves the value of an option.

*/

const char* commc_args_get_option(commc_args_parser_t* parser, const char* name) {

  commc_arg_def_t* arg_def;

  if  (!parser || !name) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  arg_def = find_arg_def(parser, name);

  if  (arg_def && arg_def->type == COMMC_ARG_OPTION) {

    if  (arg_def->is_set) {

      return arg_def->parsed_value;

    } else {

      return arg_def->default_value;

    }

  }

  return NULL;

}

/*

         commc_args_get_positional()
	       ---
	       retrieves a positional argument by its index.

*/

const char* commc_args_get_positional(commc_args_parser_t* parser, size_t index) {

  if  (!parser || !parser->positional_values || index >= parser->num_positional_parsed) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return parser->positional_values[index];

}

/*

         commc_args_print_help()
	       ---
	       prints the help message for the parser.

*/

void commc_args_print_help(commc_args_parser_t* parser, const char* program_name) {

  size_t i;
  
  if  (!parser) {

    return;

  }

  printf("usage: %s [options] [arguments]\n\n", program_name ? program_name : "program");
  printf("options:\n");

  for  (i = 0; i < parser->num_args; i++) {

    if  (parser->args[i].type == COMMC_ARG_FLAG || parser->args[i].type == COMMC_ARG_OPTION) {

      printf("  ");

      if  (parser->args[i].short_name[0] != '\0') {

        printf("-%s", parser->args[i].short_name);

      }

      if  (parser->args[i].short_name[0] != '\0' && parser->args[i].long_name[0] != '\0') {

        printf(", ");

      }

      if  (parser->args[i].long_name[0] != '\0') {

        printf("--%s", parser->args[i].long_name);

      }

      if  (parser->args[i].type == COMMC_ARG_OPTION) {

        printf(" <value>");

      }
      
      printf("    %s\n", parser->args[i].description);

    }

  }

  printf("\narguments:\n");

  for  (i = 0; i < parser->num_args; i++) {

    if  (parser->args[i].type == COMMC_ARG_POSITIONAL) {

      printf("  %s    %s\n", parser->args[i].long_name, parser->args[i].description);

    }

  }

}

/*
	==================================
             --- EOF ---
	==================================
*/
