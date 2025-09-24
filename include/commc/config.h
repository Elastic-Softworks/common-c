/* 	
   ===================================
   C O N F I G . H 
   CONFIGURATION FILE PARSER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module provides comprehensive configuration
	    file parsing with support for INI, TOML, and
	    YAML-style formats. handles nested sections,
	    arrays, and various data types.
	       
	    designed to simplify application configuration
	    management while maintaining C89 compliance
	    and cross-platform compatibility.

*/

#ifndef COMMC_CONFIG_H
#define COMMC_CONFIG_H

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
             --- CONST ---
	==================================
*/

#define COMMC_CONFIG_MAX_KEY_LENGTH      256    /* MAXIMUM KEY NAME LENGTH */
#define COMMC_CONFIG_MAX_VALUE_LENGTH    1024   /* MAXIMUM VALUE LENGTH */
#define COMMC_CONFIG_MAX_SECTION_LENGTH  256    /* MAXIMUM SECTION NAME LENGTH */
#define COMMC_CONFIG_MAX_LINE_LENGTH     2048   /* MAXIMUM LINE LENGTH */
#define COMMC_CONFIG_MAX_ENTRIES         1024   /* MAXIMUM CONFIG ENTRIES */

/* 
	==================================
             --- ENUMS ---
	==================================
*/

/*

         commc_config_format_t
	       ---
	       enumeration defining supported configuration
	       file formats for parsing and generation.

*/

typedef enum {

    COMMC_CONFIG_FORMAT_INI,      /* INI FORMAT ([SECTION] KEY=VALUE) */
    COMMC_CONFIG_FORMAT_TOML,     /* TOML FORMAT (EXTENDED INI WITH TYPES) */
    COMMC_CONFIG_FORMAT_YAML,     /* YAML FORMAT (INDENTATION-BASED) */
    COMMC_CONFIG_FORMAT_AUTO      /* AUTO-DETECT FORMAT */
    
} commc_config_format_t;

/*

         commc_config_value_type_t
	       ---
	       enumeration defining the data types that can
	       be stored and retrieved from configuration files.

*/

typedef enum {

    COMMC_CONFIG_TYPE_STRING,     /* STRING VALUE */
    COMMC_CONFIG_TYPE_INTEGER,    /* INTEGER VALUE */
    COMMC_CONFIG_TYPE_FLOAT,      /* FLOATING-POINT VALUE */
    COMMC_CONFIG_TYPE_BOOLEAN,    /* BOOLEAN VALUE (TRUE/FALSE) */
    COMMC_CONFIG_TYPE_ARRAY,      /* ARRAY OF VALUES */
    COMMC_CONFIG_TYPE_SECTION     /* NESTED SECTION */
    
} commc_config_value_type_t;

/* 
	==================================
           --- STRUCTURES ---
	==================================
*/

/*

         commc_config_value_t
	       ---
	       structure representing a single configuration
	       value with type information and data storage.

*/

typedef struct {

    commc_config_value_type_t type;                        /* VALUE TYPE */
    
    union {
    
        char*  string_value;                               /* STRING DATA */
        int    integer_value;                              /* INTEGER DATA */
        double float_value;                                /* FLOAT DATA */
        int    boolean_value;                              /* BOOLEAN DATA */
        
        struct {
        
            void** elements;                               /* ARRAY ELEMENTS */
            int    count;                                  /* ELEMENT COUNT */
            commc_config_value_type_t element_type;        /* ELEMENT TYPE */
            
        } array_value;
        
    } data;
    
} commc_config_value_t;

/*

         commc_config_entry_t
	       ---
	       structure representing a key-value pair in
	       a configuration file with section context.

*/

typedef struct {

    char section[COMMC_CONFIG_MAX_SECTION_LENGTH];         /* SECTION NAME */
    char key[COMMC_CONFIG_MAX_KEY_LENGTH];                 /* KEY NAME */
    
    commc_config_value_t value;                            /* VALUE DATA */
    
    int line_number;                                       /* SOURCE LINE */
    
} commc_config_entry_t;

/*

         commc_config_t
	       ---
	       main configuration structure containing parsed
	       configuration data with fast lookup capabilities.

*/

typedef struct {

    commc_config_format_t format;                          /* FILE FORMAT */
    
    commc_config_entry_t* entries;                         /* CONFIG ENTRIES */
    int                   entry_count;                     /* ENTRY COUNT */
    int                   entry_capacity;                  /* ALLOCATED CAPACITY */
    
    char** sections;                                       /* SECTION NAMES */
    int    section_count;                                  /* SECTION COUNT */
    
    char filename[COMMC_CONFIG_MAX_VALUE_LENGTH];          /* SOURCE FILENAME */
    
} commc_config_t;

/*

         commc_config_parser_t
	       ---
	       parser state structure for incremental
	       configuration file parsing with error tracking.

*/

typedef struct {

    commc_config_format_t format;                          /* PARSING FORMAT */
    
    char*  buffer;                                         /* FILE BUFFER */
    size_t buffer_size;                                    /* BUFFER SIZE */
    size_t buffer_pos;                                     /* CURRENT POSITION */
    
    int    line_number;                                    /* CURRENT LINE */
    int    column_number;                                  /* CURRENT COLUMN */
    
    char   current_section[COMMC_CONFIG_MAX_SECTION_LENGTH]; /* CURRENT SECTION */
    
    int    error_line;                                     /* ERROR LINE */
    char   error_message[COMMC_CONFIG_MAX_VALUE_LENGTH];   /* ERROR MESSAGE */
    
} commc_config_parser_t;

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_config_create()
	       ---
	       creates and initializes a new configuration
	       structure with default settings and allocated
	       storage for entries.

*/

commc_error_t commc_config_create(commc_config_t** config);

/*

         commc_config_destroy()
	       ---
	       destroys a configuration structure and frees
	       all allocated memory including entries and
	       string values.

*/

void commc_config_destroy(commc_config_t* config);

/*

         commc_config_load()
	       ---
	       loads and parses a configuration file with
	       automatic format detection or explicit format
	       specification.

*/

commc_error_t commc_config_load(commc_config_t*       config,
                                const char*           filename,
                                commc_config_format_t format);

/*

         commc_config_load_string()
	       ---
	       parses configuration data from a string buffer
	       instead of a file, useful for embedded or
	       generated configurations.

*/

commc_error_t commc_config_load_string(commc_config_t*       config,
                                       const char*           data,
                                       size_t                data_size,
                                       commc_config_format_t format);

/*

         commc_config_save()
	       ---
	       saves configuration data to a file in the
	       specified format with proper formatting
	       and comments.

*/

commc_error_t commc_config_save(commc_config_t*       config,
                                const char*           filename,
                                commc_config_format_t format);

/* 
	==================================
           --- ACCESSORS ---
	==================================
*/

/*

         commc_config_get_string()
	       ---
	       retrieves a string value from the configuration
	       by section and key, with optional default value.

*/

const char* commc_config_get_string(commc_config_t* config,
                                    const char*     section,
                                    const char*     key,
                                    const char*     default_value);

/*

         commc_config_get_integer()
	       ---
	       retrieves an integer value from the configuration
	       with automatic type conversion and default value.

*/

int commc_config_get_integer(commc_config_t* config,
                             const char*     section,
                             const char*     key,
                             int             default_value);

/*

         commc_config_get_float()
	       ---
	       retrieves a floating-point value from the
	       configuration with type conversion support.

*/

double commc_config_get_float(commc_config_t* config,
                              const char*     section,
                              const char*     key,
                              double          default_value);

/*

         commc_config_get_boolean()
	       ---
	       retrieves a boolean value from the configuration
	       supporting various true/false representations.

*/

int commc_config_get_boolean(commc_config_t* config,
                             const char*     section,
                             const char*     key,
                             int             default_value);

/*

         commc_config_get_array_count()
	       ---
	       returns the number of elements in an array
	       configuration value.

*/

int commc_config_get_array_count(commc_config_t* config,
                                 const char*     section,
                                 const char*     key);

/*

         commc_config_get_array_string()
	       ---
	       retrieves a string element from an array
	       configuration value by index.

*/

const char* commc_config_get_array_string(commc_config_t* config,
                                          const char*     section,
                                          const char*     key,
                                          int             index);

/* 
	==================================
            --- SETTERS ---
	==================================
*/

/*

         commc_config_set_string()
	       ---
	       sets a string value in the configuration,
	       creating the section and key if they don't exist.

*/

commc_error_t commc_config_set_string(commc_config_t* config,
                                      const char*     section,
                                      const char*     key,
                                      const char*     value);

/*

         commc_config_set_integer()
	       ---
	       sets an integer value in the configuration
	       with automatic type conversion to string.

*/

commc_error_t commc_config_set_integer(commc_config_t* config,
                                       const char*     section,
                                       const char*     key,
                                       int             value);

/*

         commc_config_set_float()
	       ---
	       sets a floating-point value in the configuration
	       with precision control and formatting.

*/

commc_error_t commc_config_set_float(commc_config_t* config,
                                     const char*     section,
                                     const char*     key,
                                     double          value);

/*

         commc_config_set_boolean()
	       ---
	       sets a boolean value in the configuration
	       using standard true/false representation.

*/

commc_error_t commc_config_set_boolean(commc_config_t* config,
                                       const char*     section,
                                       const char*     key,
                                       int             value);

/* 
	==================================
           --- UTILITIES ---
	==================================
*/

/*

         commc_config_has_section()
	       ---
	       checks if a configuration section exists
	       in the loaded configuration data.

*/

int commc_config_has_section(commc_config_t* config,
                             const char*     section);

/*

         commc_config_has_key()
	       ---
	       checks if a specific key exists within
	       a configuration section.

*/

int commc_config_has_key(commc_config_t* config,
                         const char*     section,
                         const char*     key);

/*

         commc_config_remove_key()
	       ---
	       removes a key-value pair from the configuration,
	       preserving other entries in the same section.

*/

commc_error_t commc_config_remove_key(commc_config_t* config,
                                      const char*     section,
                                      const char*     key);

/*

         commc_config_remove_section()
	       ---
	       removes an entire section and all its keys
	       from the configuration data.

*/

commc_error_t commc_config_remove_section(commc_config_t* config,
                                          const char*     section);

/*

         commc_config_get_sections()
	       ---
	       retrieves a list of all section names in
	       the configuration for enumeration purposes.

*/

const char** commc_config_get_sections(commc_config_t* config,
                                       int*            section_count);

/*

         commc_config_get_keys()
	       ---
	       retrieves a list of all key names within
	       a specific section for enumeration.

*/

const char** commc_config_get_keys(commc_config_t* config,
                                   const char*     section,
                                   int*            key_count);

/* 
	==================================
           --- VALIDATION ---
	==================================
*/

/*

         commc_config_validate()
	       ---
	       validates configuration data against a schema
	       or set of rules, reporting any inconsistencies
	       or missing required values.

*/

commc_error_t commc_config_validate(commc_config_t* config,
                                    const char*     schema_file);

/*

         commc_config_merge()
	       ---
	       merges two configuration structures, with
	       the second configuration taking precedence
	       for conflicting keys.

*/

commc_error_t commc_config_merge(commc_config_t*       base_config,
                                 const commc_config_t* override_config);

/*

         commc_config_clone()
	       ---
	       creates a deep copy of a configuration
	       structure with all entries and values.

*/

commc_error_t commc_config_clone(const commc_config_t* source,
                                 commc_config_t**      destination);

/* 
	==================================
           --- FORMATTING ---
	==================================
*/

/*

         commc_config_format_detect()
	       ---
	       automatically detects the configuration file
	       format by analyzing file content and structure.

*/

commc_config_format_t commc_config_format_detect(const char* data,
                                                  size_t      data_size);

/*

         commc_config_format_to_string()
	       ---
	       converts configuration data to a formatted
	       string representation in the specified format.

*/

commc_error_t commc_config_format_to_string(commc_config_t*       config,
                                            commc_config_format_t format,
                                            char**                output,
                                            size_t*               output_size);

/*

         commc_config_get_error_info()
	       ---
	       retrieves detailed error information from
	       the last parsing operation including line
	       numbers and error descriptions.

*/

const char* commc_config_get_error_info(commc_config_t* config,
                                        int*            error_line,
                                        int*            error_column);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_CONFIG_H */

/* 
	==================================
             --- EOF ---
	==================================
*/