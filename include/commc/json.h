/* 	
   ===================================
   J S O N  P A R S E R  A N D  G E N E R A T O R
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- JSON ETHOS ---

	    JSON (JavaScript Object Notation) is a lightweight,
	    text-based data interchange format. this implementation
	    provides a complete parser with validation, error
	    reporting, and generator for creating JSON documents.
	    
	    the parser supports streaming for large documents and
	    provides detailed error messages with line/column
	    information for debugging malformed JSON.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_JSON_H
#define COMMC_JSON_H

#include  <stddef.h>   /* for size_t */
#include  <stdint.h>   /* for uint8_t */

#include  "commc/error.h"

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

/* JSON parsing limits */

#define COMMC_JSON_MAX_DEPTH               64      /* maximum nesting depth */
#define COMMC_JSON_MAX_STRING_LENGTH       65536   /* maximum string length */
#define COMMC_JSON_MAX_NUMBER_LENGTH       32      /* maximum number length */

/* JSON value types */

#define COMMC_JSON_TYPE_NULL               0       /* null value */
#define COMMC_JSON_TYPE_BOOLEAN            1       /* true/false */
#define COMMC_JSON_TYPE_NUMBER             2       /* numeric value */
#define COMMC_JSON_TYPE_STRING             3       /* string value */
#define COMMC_JSON_TYPE_ARRAY              4       /* array container */
#define COMMC_JSON_TYPE_OBJECT             5       /* object container */

/* JSON parsing modes */

#define COMMC_JSON_PARSE_STRICT            0       /* strict JSON parsing */
#define COMMC_JSON_PARSE_RELAXED           1       /* allow comments, trailing commas */

/* JSON generation modes */

#define COMMC_JSON_FORMAT_COMPACT          0       /* compact output */
#define COMMC_JSON_FORMAT_PRETTY           1       /* pretty-printed output */

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_json_value_t
	       ---
	       represents a JSON value of any type with
	       associated data and metadata.

*/

typedef struct commc_json_value {

  int type;                         /* value type */
  
  union {
  
    int boolean;                    /* boolean value */
    double number;                  /* numeric value */
    char* string;                   /* string value */
    
    struct {
    
      struct commc_json_value** items;    /* array items */
      size_t count;                       /* number of items */
      size_t capacity;                    /* allocated capacity */
      
    } array;
    
    struct {
    
      char** keys;                        /* object keys */
      struct commc_json_value** values;   /* object values */
      size_t count;                       /* number of pairs */
      size_t capacity;                    /* allocated capacity */
      
    } object;
    
  } data;

} commc_json_value_t;

/*

         commc_json_error_t
	       ---
	       detailed error information for JSON parsing
	       failures with location and context.

*/

typedef struct {

  int code;                         /* error code */
  char* message;                    /* error description */
  
  size_t line;                      /* error line number */
  size_t column;                    /* error column number */
  size_t position;                  /* byte position in input */
  
  char* context;                    /* surrounding text context */

} commc_json_error_t;

/*

         commc_json_parser_t
	       ---
	       JSON parser context maintaining state for
	       streaming and incremental parsing.

*/

typedef struct {

  const char* input;                /* input JSON text */
  size_t input_size;                /* size of input */
  size_t position;                  /* current parse position */
  
  size_t line;                      /* current line number */
  size_t column;                    /* current column number */
  
  int parse_mode;                   /* parsing mode flags */
  int max_depth;                    /* maximum nesting depth */
  int current_depth;                /* current nesting depth */
  
  commc_json_error_t error;         /* last error information */

} commc_json_parser_t;

/*

         commc_json_generator_t
	       ---
	       JSON generator context for creating formatted
	       JSON output with configurable styling.

*/

typedef struct {

  char* output;                     /* output buffer */
  size_t output_size;               /* current output size */
  size_t output_capacity;           /* output buffer capacity */
  
  int format_mode;                  /* formatting mode */
  int indent_level;                 /* current indentation level */
  size_t indent_size;               /* spaces per indent level */
  
  int pretty_print;                 /* enable pretty printing */
  int escape_unicode;               /* escape unicode characters */

} commc_json_generator_t;

/*

         commc_json_path_t
	       ---
	       represents a path to a specific value within
	       a JSON document for querying and modification.

*/

typedef struct {

  char** segments;                  /* path segments */
  size_t segment_count;             /* number of segments */
  int* indices;                     /* array indices (or -1 for objects) */

} commc_json_path_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/* value creation and destruction */

commc_json_value_t* commc_json_value_create_null(void);

commc_json_value_t* commc_json_value_create_boolean(int value);

commc_json_value_t* commc_json_value_create_number(double value);

commc_json_value_t* commc_json_value_create_string(const char* value);

commc_json_value_t* commc_json_value_create_array(void);

commc_json_value_t* commc_json_value_create_object(void);

void commc_json_value_destroy(commc_json_value_t* value);

/* value manipulation */

int commc_json_array_add(commc_json_value_t* array, commc_json_value_t* item);

int commc_json_object_set(commc_json_value_t* object, 
                          const char* key, 
                          commc_json_value_t* value);

commc_json_value_t* commc_json_object_get(const commc_json_value_t* object, 
                                          const char* key);

int commc_json_object_remove(commc_json_value_t* object, const char* key);

/* parser functions */

commc_json_parser_t* commc_json_parser_create(void);

void commc_json_parser_destroy(commc_json_parser_t* parser);

void commc_json_parser_set_mode(commc_json_parser_t* parser, int mode);

void commc_json_parser_set_max_depth(commc_json_parser_t* parser, int depth);

/* parsing functions */

commc_json_value_t* commc_json_parse(const char* json_text);

commc_json_value_t* commc_json_parse_with_parser(commc_json_parser_t* parser,
                                                  const char* json_text,
                                                  size_t json_size);

int commc_json_parse_streaming(commc_json_parser_t* parser,
                               const char* chunk,
                               size_t chunk_size,
                               commc_json_value_t** result);

/* generator functions */

commc_json_generator_t* commc_json_generator_create(void);

void commc_json_generator_destroy(commc_json_generator_t* generator);

void commc_json_generator_set_format(commc_json_generator_t* generator,
                                     int format_mode);

void commc_json_generator_set_indent(commc_json_generator_t* generator,
                                     size_t indent_size);

/* generation functions */

char* commc_json_generate(const commc_json_value_t* value);

int commc_json_generate_to_buffer(const commc_json_value_t* value,
                                  char* buffer,
                                  size_t buffer_size,
                                  size_t* output_size);

int commc_json_generate_with_generator(commc_json_generator_t* generator,
                                       const commc_json_value_t* value,
                                       char** output,
                                       size_t* output_size);

/* validation functions */

int commc_json_validate(const char* json_text);

int commc_json_validate_value(const commc_json_value_t* value);

/* query functions */

commc_json_value_t* commc_json_query(const commc_json_value_t* root,
                                      const char* path);

commc_json_path_t* commc_json_path_parse(const char* path_string);

void commc_json_path_destroy(commc_json_path_t* path);

/* utility functions */

int commc_json_value_equals(const commc_json_value_t* a,
                            const commc_json_value_t* b);

commc_json_value_t* commc_json_value_clone(const commc_json_value_t* source);

size_t commc_json_value_size(const commc_json_value_t* value);

/* type checking functions */

int commc_json_is_null(const commc_json_value_t* value);

int commc_json_is_boolean(const commc_json_value_t* value);

int commc_json_is_number(const commc_json_value_t* value);

int commc_json_is_string(const commc_json_value_t* value);

int commc_json_is_array(const commc_json_value_t* value);

int commc_json_is_object(const commc_json_value_t* value);

/* value extraction functions */

int commc_json_get_boolean(const commc_json_value_t* value);

double commc_json_get_number(const commc_json_value_t* value);

const char* commc_json_get_string(const commc_json_value_t* value);

size_t commc_json_array_size(const commc_json_value_t* array);

commc_json_value_t* commc_json_array_get(const commc_json_value_t* array,
                                          size_t index);

size_t commc_json_object_size(const commc_json_value_t* object);

/* error handling */

const commc_json_error_t* commc_json_parser_get_error(const commc_json_parser_t* parser);

void commc_json_error_destroy(commc_json_error_t* error);

/* 
	==================================
             --- EOF ---
	==================================
*/

#endif /* COMMC_JSON_H */