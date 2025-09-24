/*

   ===================================
   C O M M C   -   C S V   P A R S E R
   CSV PARSING AND WRITING SYSTEM
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_CSV_H
#define COMMC_CSV_H

#include "error.h"
#include <stdio.h>

/* 
	==================================
          --- TYPE DEFS ---
	==================================
*/

/*

	commc_csv_config_t
	---
	configuration structure for CSV parser behavior.
	allows customization of delimiter, quote character,
	escape sequences, and parsing modes to handle
	various CSV dialects and formatting requirements.

*/

typedef struct {

    char delimiter;       /* FIELD DELIMITER (USUALLY COMMA) */
    char quote;          /* QUOTE CHARACTER FOR TEXT FIELDS */
    char escape;         /* ESCAPE CHARACTER FOR QUOTES */
    
    int  skip_empty_lines;    /* SKIP BLANK LINES IN INPUT */
    int  trim_whitespace;     /* TRIM LEADING/TRAILING SPACES */
    int  allow_multiline;     /* ALLOW QUOTED MULTILINE FIELDS */
    int  strict_quotes;       /* REQUIRE QUOTES FOR ALL TEXT */

} commc_csv_config_t;

/*

	commc_csv_field_t
	---
	represents a single field in a CSV record.
	stores both the raw text content and metadata
	about the field's parsing characteristics for
	proper reconstruction and validation.

*/

typedef struct {

    char* data;          /* FIELD CONTENT STRING */
    int   length;        /* LENGTH OF FIELD DATA */
    int   quoted;        /* WAS FIELD QUOTED IN SOURCE */
    int   escaped;       /* CONTAINS ESCAPE SEQUENCES */

} commc_csv_field_t;

/*

	commc_csv_record_t
	---
	represents a single row/record in CSV data.
	contains array of fields with dynamic sizing
	and metadata for record reconstruction and
	validation during parsing and writing operations.

*/

typedef struct {

    commc_csv_field_t* fields;    /* ARRAY OF FIELD STRUCTURES */
    int                count;     /* NUMBER OF FIELDS IN RECORD */
    int                capacity;  /* ALLOCATED FIELD CAPACITY */
    int                line_num;  /* SOURCE LINE NUMBER */

} commc_csv_record_t;

/*

	commc_csv_parser_t
	---
	streaming CSV parser context that maintains
	state for incremental parsing of large files.
	provides buffered input processing with proper
	error tracking and memory management.

*/

typedef struct {

    commc_csv_config_t config;     /* PARSER CONFIGURATION */
    
    char*              buffer;     /* INPUT BUFFER FOR STREAMING */
    int                buf_size;   /* BUFFER SIZE IN BYTES */
    int                buf_pos;    /* CURRENT BUFFER POSITION */
    int                buf_len;    /* VALID DATA LENGTH IN BUFFER */
    
    FILE*              input;      /* INPUT FILE STREAM */
    int                    line_num;   /* CURRENT LINE NUMBER */
    int                    col_num;    /* CURRENT COLUMN POSITION */
    
    commc_error_context_t  error;      /* ERROR CONTEXT INFORMATION */

} commc_csv_parser_t;

/*

	commc_csv_writer_t
	---
	streaming CSV writer context for efficient
	output generation with proper escaping and
	formatting. maintains consistent output style
	and handles large dataset writing with buffering.

*/

typedef struct {

    commc_csv_config_t config;     /* WRITER CONFIGURATION */
    
    char*              buffer;     /* OUTPUT BUFFER FOR BATCHING */
    int                buf_size;   /* BUFFER SIZE IN BYTES */
    int                buf_pos;    /* CURRENT BUFFER POSITION */
    
    FILE*                  output;     /* OUTPUT FILE STREAM */
    int                    records_written; /* COUNT OF OUTPUT RECORDS */
    
    commc_error_context_t  error;      /* ERROR CONTEXT INFORMATION */

} commc_csv_writer_t;

/* 
	==================================
          --- PARSER API ---
	==================================
*/

/*

         commc_csv_parser_create()
	       ---
	       creates and initializes a new CSV parser with
	       the specified configuration. allocates necessary
	       buffers for streaming input and sets up error
	       tracking context for detailed parsing feedback.

*/

commc_csv_parser_t* commc_csv_parser_create(const commc_csv_config_t* config, int buffer_size);

/*

         commc_csv_parser_destroy()
	       ---
	       releases all memory and resources associated
	       with a CSV parser. properly closes any open
	       file handles and clears error contexts to
	       prevent resource leaks in long-running applications.

*/

void commc_csv_parser_destroy(commc_csv_parser_t* parser);

/*

         commc_csv_parser_open_file()
	       ---
	       associates a CSV parser with an input file
	       for streaming processing. handles file opening
	       errors and initializes parsing state for
	       sequential record processing from disk.

*/

int commc_csv_parser_open_file(commc_csv_parser_t* parser, const char* filename);

/*

         commc_csv_parser_set_string()
	       ---
	       configures parser to read from an in-memory
	       string buffer instead of a file. useful for
	       processing CSV data from network requests or
	       embedded string literals in applications.

*/

int commc_csv_parser_set_string(commc_csv_parser_t* parser, const char* data, int length);

/*

         commc_csv_parser_next_record()
	       ---
	       reads and parses the next CSV record from
	       the input source. handles field parsing,
	       quote processing, escape sequence interpretation,
	       and error detection with detailed context.

*/

commc_csv_record_t* commc_csv_parser_next_record(commc_csv_parser_t* parser);

/*

         commc_csv_parser_peek_record()
	       ---
	       examines the next record without consuming it
	       from the input stream. allows applications
	       to preview upcoming data for conditional
	       processing or validation before actual parsing.

*/

commc_csv_record_t* commc_csv_parser_peek_record(commc_csv_parser_t* parser);

/*

         commc_csv_parser_skip_record()
	       ---
	       advances past the current record without
	       parsing its contents. efficient for skipping
	       header rows or filtering unwanted records
	       during streaming processing operations.

*/

int commc_csv_parser_skip_record(commc_csv_parser_t* parser);

/*

         commc_csv_parser_get_error()
	       ---
	       retrieves detailed error information from
	       the parser context. provides line/column
	       position, error codes, and descriptive
	       messages for troubleshooting parsing issues.

*/

const commc_error_context_t* commc_csv_parser_get_error(const commc_csv_parser_t* parser);

/* 
	==================================
          --- WRITER API ---
	==================================
*/

/*

         commc_csv_writer_create()
	       ---
	       creates and initializes a new CSV writer with
	       specified configuration and output buffering.
	       sets up proper escaping rules and formatting
	       options for consistent output generation.

*/

commc_csv_writer_t* commc_csv_writer_create(const commc_csv_config_t* config, int buffer_size);

/*

         commc_csv_writer_destroy()
	       ---
	       releases all resources associated with a CSV
	       writer. flushes any pending output to ensure
	       complete data writing and closes file handles
	       to prevent resource leaks in applications.

*/

void commc_csv_writer_destroy(commc_csv_writer_t* writer);

/*

         commc_csv_writer_open_file()
	       ---
	       associates a CSV writer with an output file
	       for record writing. handles file creation and
	       opening errors while setting up buffered
	       output for efficient large dataset processing.

*/

int commc_csv_writer_open_file(commc_csv_writer_t* writer, const char* filename);

/*

         commc_csv_writer_set_string()
	       ---
	       configures writer to output to a dynamically
	       growing string buffer. useful for generating
	       CSV data in memory for network transmission
	       or embedding in other data structures.

*/

int commc_csv_writer_set_string(commc_csv_writer_t* writer, char** output, int* output_size);

/*

         commc_csv_writer_write_record()
	       ---
	       writes a complete CSV record to the output
	       destination. handles proper field escaping,
	       quote insertion, delimiter placement, and
	       line termination according to configuration.

*/

int commc_csv_writer_write_record(commc_csv_writer_t* writer, const commc_csv_record_t* record);

/*

         commc_csv_writer_write_fields()
	       ---
	       writes a record from an array of field strings.
	       automatically determines quoting requirements
	       and applies proper escape sequences for
	       safe CSV output with custom field arrays.

*/

int commc_csv_writer_write_fields(commc_csv_writer_t* writer, char** fields, int count);

/*

         commc_csv_writer_write_header()
	       ---
	       writes column header names as the first record
	       in CSV output. applies consistent formatting
	       and ensures proper field separation for
	       well-structured CSV file generation.

*/

int commc_csv_writer_write_header(commc_csv_writer_t* writer, char** headers, int count);

/*

         commc_csv_writer_flush()
	       ---
	       forces all buffered output to be written to
	       the destination. ensures data persistence
	       and enables real-time CSV streaming for
	       applications requiring immediate output visibility.

*/

int commc_csv_writer_flush(commc_csv_writer_t* writer);

/*

         commc_csv_writer_get_error()
	       ---
	       retrieves error information from writer context.
	       provides details about write failures, disk
	       space issues, or formatting problems during
	       CSV generation and output operations.

*/

const commc_error_context_t* commc_csv_writer_get_error(const commc_csv_writer_t* writer);

/* 
	==================================
         --- RECORD API ---
	==================================
*/

/*

         commc_csv_record_create()
	       ---
	       creates a new empty CSV record with initial
	       field capacity. allocates memory for field
	       storage and initializes metadata for proper
	       record manipulation and memory management.

*/

commc_csv_record_t* commc_csv_record_create(int initial_capacity);

/*

         commc_csv_record_destroy()
	       ---
	       releases all memory associated with a CSV
	       record including field data and the record
	       structure itself. prevents memory leaks
	       during record processing and manipulation.

*/

void commc_csv_record_destroy(commc_csv_record_t* record);

/*

         commc_csv_record_add_field()
	       ---
	       appends a new field to a CSV record with
	       automatic capacity expansion. copies field
	       data and maintains proper metadata for
	       quote and escape sequence requirements.

*/

int commc_csv_record_add_field(commc_csv_record_t* record, const char* data, int length, int quoted);

/*

         commc_csv_record_set_field()
	       ---
	       modifies an existing field at a specific
	       index within a CSV record. handles memory
	       reallocation for variable-length field data
	       and updates associated metadata appropriately.

*/

int commc_csv_record_set_field(commc_csv_record_t* record, int index, const char* data, int length, int quoted);

/*

         commc_csv_record_get_field()
	       ---
	       retrieves field data from a record at the
	       specified index. returns field structure
	       with content, length, and formatting metadata
	       for safe access to parsed CSV information.

*/

const commc_csv_field_t* commc_csv_record_get_field(const commc_csv_record_t* record, int index);

/*

         commc_csv_record_get_field_string()
	       ---
	       extracts field content as a null-terminated
	       string. handles memory allocation for string
	       conversion and provides convenient access
	       to field data for string-based operations.

*/

char* commc_csv_record_get_field_string(const commc_csv_record_t* record, int index);

/*

         commc_csv_record_find_field()
	       ---
	       searches for a field by header name when
	       working with named columns. returns field
	       index or -1 if not found, enabling column-
	       based access to CSV data structures.

*/

int commc_csv_record_find_field(const commc_csv_record_t* record, char** headers, const char* name);

/*

         commc_csv_record_clone()
	       ---
	       creates a deep copy of a CSV record including
	       all field data and metadata. useful for
	       record caching, modification without affecting
	       original data, and multi-threaded processing.

*/

commc_csv_record_t* commc_csv_record_clone(const commc_csv_record_t* record);

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_csv_config_default()
	       ---
	       returns a standard CSV configuration with
	       common settings: comma delimiter, double-quote
	       character, and typical parsing behavior for
	       RFC 4180 compliant CSV processing.

*/

commc_csv_config_t commc_csv_config_default(void);

/*

         commc_csv_config_excel()
	       ---
	       returns configuration optimized for Microsoft
	       Excel CSV format compatibility. handles
	       Excel-specific quirks and formatting requirements
	       for seamless spreadsheet integration.

*/

commc_csv_config_t commc_csv_config_excel(void);

/*

         commc_csv_config_tab()
	       ---
	       returns configuration for tab-separated values
	       (TSV) format processing. uses tab delimiters
	       with appropriate quoting rules for TSV
	       file handling and data exchange.

*/

commc_csv_config_t commc_csv_config_tab(void);

/*

         commc_csv_escape_field()
	       ---
	       determines if a field requires quoting and
	       escaping based on content and configuration.
	       analyzes field data for special characters
	       and returns appropriate formatting flags.

*/

int commc_csv_escape_field(const char* data, int length, const commc_csv_config_t* config);

/*

         commc_csv_unescape_field()
	       ---
	       processes escaped field data to restore
	       original content by removing quotes and
	       interpreting escape sequences according
	       to CSV formatting rules and configuration.

*/

char* commc_csv_unescape_field(const char* data, int length, const commc_csv_config_t* config);

/*

         commc_csv_validate_record()
	       ---
	       checks a CSV record for consistency and
	       proper formatting. verifies field counts,
	       escape sequences, and adherence to
	       configuration rules for data validation.

*/

int commc_csv_validate_record(const commc_csv_record_t* record, const commc_csv_config_t* config);

/*

         commc_csv_count_records()
	       ---
	       efficiently counts the number of records
	       in a CSV file without full parsing. uses
	       streaming techniques to handle large files
	       while providing accurate record counts.

*/

int commc_csv_count_records(const char* filename, const commc_csv_config_t* config);

/*

         commc_csv_get_headers()
	       ---
	       extracts header row from a CSV file as
	       an array of strings. useful for dynamic
	       column discovery and schema validation
	       in data processing applications.

*/

char** commc_csv_get_headers(const char* filename, const commc_csv_config_t* config, int* count);

/* 
	==================================
           --- EOF ---
	==================================
*/

#endif /* COMMC_CSV_H */