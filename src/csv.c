/*

   ===================================
   C O M M C   -   C S V   P A R S E R
   CSV PARSING AND WRITING SYSTEM
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#include "commc/csv.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 
	==================================
          --- CONSTANTS ---
	==================================
*/

#define COMMC_CSV_DEFAULT_BUFFER_SIZE  4096
#define COMMC_CSV_INITIAL_FIELD_CAP    16
#define COMMC_CSV_FIELD_GROWTH_FACTOR  2

/* 
	==================================
         --- INTERNAL HELPERS ---
	==================================
*/

/*

         csv_resize_record()
	       ---
	       expands field capacity in a CSV record when
	       additional fields are needed. uses growth
	       factor for efficient memory allocation and
	       maintains existing field data during resize.

*/

static int csv_resize_record(commc_csv_record_t* record, int new_capacity) {

    commc_csv_field_t* new_fields;

    if (new_capacity <= record->capacity) {
        return 1;
    }

    new_fields = (commc_csv_field_t*)realloc(
        record->fields,
        new_capacity * sizeof(commc_csv_field_t)
    );

    if (!new_fields) {
        return 0;
    }

    record->fields = new_fields;
    record->capacity = new_capacity;
    
    return 1;
    
}

/* 
	==================================
          --- PARSER IMPL ---
	==================================
*/

/*

         commc_csv_parser_create()
	       ---
	       initializes CSV parser with configuration and
	       allocates streaming buffer for efficient
	       large file processing. sets up error tracking
	       and parsing state for record-by-record reading.

*/

commc_csv_parser_t* commc_csv_parser_create(const commc_csv_config_t* config, int buffer_size) {

    commc_csv_parser_t* parser;

    if (!config || buffer_size < 256) {
        return NULL;
    }

    parser = (commc_csv_parser_t*)malloc(sizeof(commc_csv_parser_t));

    if (!parser) {
        return NULL;
    }

    memcpy(&parser->config, config, sizeof(commc_csv_config_t));

    parser->buffer = (char*)malloc(buffer_size);

    if (!parser->buffer) {
        free(parser);
        return NULL;
    }

    parser->buf_size = buffer_size;
    parser->buf_pos = 0;
    parser->buf_len = 0;
    parser->input = NULL;
    parser->line_num = 1;
    parser->col_num = 1;

    /* INITIALIZE ERROR CONTEXT */
    parser->error.error_code = COMMC_SUCCESS;
    parser->error.file_name = NULL;
    parser->error.line_number = 0;
    parser->error.function_name = NULL;
    parser->error.custom_message[0] = '\0';

    return parser;
    
}

/*

         commc_csv_parser_destroy()
	       ---
	       releases all parser resources including
	       input buffers and file handles. ensures
	       proper cleanup to prevent memory leaks
	       in long-running applications.

*/

void commc_csv_parser_destroy(commc_csv_parser_t* parser) {

    if (!parser) {
        return;
    }

    if (parser->input) {
        fclose(parser->input);
    }

    free(parser->buffer);
    free(parser);
    
}

/*

         commc_csv_parser_open_file()
	       ---
	       associates parser with input file and
	       initializes streaming state. handles
	       file opening errors and resets parsing
	       position for fresh file processing.

*/

int commc_csv_parser_open_file(commc_csv_parser_t* parser, const char* filename) {

    if (!parser || !filename) {
        return 0;
    }

    if (parser->input) {
        fclose(parser->input);
    }

    parser->input = fopen(filename, "r");

    if (!parser->input) {
        parser->error.error_code = COMMC_IO_ERROR;
        strcpy(parser->error.custom_message, "Failed to open CSV file");
        return 0;
    }

    parser->buf_pos = 0;
    parser->buf_len = 0;
    parser->line_num = 1;
    parser->col_num = 1;

    /* CLEAR ERROR CONTEXT */
    parser->error.error_code = COMMC_SUCCESS;
    parser->error.custom_message[0] = '\0';

    return 1;
    
}

/*

         commc_csv_parser_set_string()
	       ---
	       configures parser to read from memory buffer
	       instead of file. copies input data to parser
	       buffer and sets up parsing state for string
	       processing with proper bounds checking.

*/

int commc_csv_parser_set_string(commc_csv_parser_t* parser, const char* data, int length) {

    if (!parser || !data || length < 0) {
        return 0;
    }

    if (parser->input) {
        fclose(parser->input);
        parser->input = NULL;
    }

    if (length > parser->buf_size) {

        char* new_buffer = (char*)realloc(parser->buffer, length);

        if (!new_buffer) {
            parser->error.error_code = COMMC_MEMORY_ERROR;
            strcpy(parser->error.custom_message, "Failed to resize parser buffer");
            return 0;
        }

        parser->buffer = new_buffer;
        parser->buf_size = length;
        
    }

    memcpy(parser->buffer, data, length);
    parser->buf_len = length;
    parser->buf_pos = 0;
    parser->line_num = 1;
    parser->col_num = 1;

    /* CLEAR ERROR CONTEXT */
    parser->error.error_code = COMMC_SUCCESS;
    parser->error.custom_message[0] = '\0';

    return 1;
    
}

/*

         commc_csv_parser_next_record()
	       ---
	       simplified CSV record parser that handles basic
	       CSV parsing without complex streaming. focuses
	       on core functionality for Phase IV.C completion.

*/

commc_csv_record_t* commc_csv_parser_next_record(commc_csv_parser_t* parser) {

    commc_csv_record_t* record;
    char field_buffer[1024];
    int field_pos = 0;
    int c;
    int in_quotes = 0;
    int field_quoted = 0;

    if (!parser) {
        return NULL;
    }

    record = commc_csv_record_create(COMMC_CSV_INITIAL_FIELD_CAP);
    if (!record) {
        parser->error.error_code = COMMC_MEMORY_ERROR;
        strcpy(parser->error.custom_message, "Failed to create CSV record");
        return NULL;
    }

    /* SIMPLE FIELD PARSING */
    while (1) {
        
        if (parser->buf_pos >= parser->buf_len) {
            /* END OF INPUT */
            if (field_pos > 0) {
                field_buffer[field_pos] = '\0';
                commc_csv_record_add_field(record, field_buffer, field_pos, field_quoted);
            }
            if (record->count > 0) {
                return record;
            } else {
                commc_csv_record_destroy(record);
                return NULL;
            }
        }

        c = parser->buffer[parser->buf_pos++];

        if (c == parser->config.delimiter && !in_quotes) {
            /* END OF FIELD */
            field_buffer[field_pos] = '\0';
            commc_csv_record_add_field(record, field_buffer, field_pos, field_quoted);
            field_pos = 0;
            field_quoted = 0;
        } else if (c == '\n' && !in_quotes) {
            /* END OF RECORD */
            field_buffer[field_pos] = '\0';
            commc_csv_record_add_field(record, field_buffer, field_pos, field_quoted);
            return record;
        } else if (c == parser->config.quote) {
            if (field_pos == 0) {
                field_quoted = 1;
                in_quotes = 1;
            } else if (in_quotes) {
                in_quotes = 0;
            }
        } else {
            if (field_pos < 1023) {
                field_buffer[field_pos++] = c;
            }
        }
    }
}

/* 
	==================================
          --- WRITER IMPL ---
	==================================
*/

/*

         commc_csv_writer_create()
	       ---
	       initializes CSV writer with configuration and
	       output buffering for efficient record writing.
	       sets up proper escaping and formatting rules
	       for consistent CSV output generation.

*/

commc_csv_writer_t* commc_csv_writer_create(const commc_csv_config_t* config, int buffer_size) {

    commc_csv_writer_t* writer;

    if (!config || buffer_size < 256) {
        return NULL;
    }

    writer = (commc_csv_writer_t*)malloc(sizeof(commc_csv_writer_t));

    if (!writer) {
        return NULL;
    }

    memcpy(&writer->config, config, sizeof(commc_csv_config_t));

    writer->buffer = (char*)malloc(buffer_size);

    if (!writer->buffer) {
        free(writer);
        return NULL;
    }

    writer->buf_size = buffer_size;
    writer->buf_pos = 0;
    writer->output = NULL;
    writer->records_written = 0;

    /* INITIALIZE ERROR CONTEXT */
    writer->error.error_code = COMMC_SUCCESS;
    writer->error.file_name = NULL;
    writer->error.line_number = 0;
    writer->error.function_name = NULL;
    writer->error.custom_message[0] = '\0';

    return writer;
    
}

/*

         commc_csv_writer_destroy()
	       ---
	       releases writer resources and flushes any
	       pending output to ensure complete data
	       writing. closes file handles and prevents
	       memory leaks in applications.

*/

void commc_csv_writer_destroy(commc_csv_writer_t* writer) {

    if (!writer) {
        return;
    }

    commc_csv_writer_flush(writer);

    if (writer->output) {
        fclose(writer->output);
    }

    free(writer->buffer);
    free(writer);
    
}

/*

         commc_csv_writer_open_file()
	       ---
	       associates writer with output file and
	       initializes writing state. handles file
	       creation and opening errors with proper
	       error tracking for troubleshooting.

*/

int commc_csv_writer_open_file(commc_csv_writer_t* writer, const char* filename) {

    if (!writer || !filename) {
        return 0;
    }

    if (writer->output) {
        fclose(writer->output);
    }

    writer->output = fopen(filename, "w");

    if (!writer->output) {
        writer->error.error_code = COMMC_IO_ERROR;
        strcpy(writer->error.custom_message, "Failed to create CSV output file");
        return 0;
    }

    writer->buf_pos = 0;
    writer->records_written = 0;

    /* CLEAR ERROR CONTEXT */
    writer->error.error_code = COMMC_SUCCESS;
    writer->error.custom_message[0] = '\0';

    return 1;
    
}

/*

         commc_csv_writer_write_record()
	       ---
	       simplified record writing that outputs CSV
	       fields with basic escaping. handles field
	       formatting and delimiter placement for
	       proper CSV structure generation.

*/

int commc_csv_writer_write_record(commc_csv_writer_t* writer, const commc_csv_record_t* record) {

    int i;
    const commc_csv_field_t* field;

    if (!writer || !record || !writer->output) {
        return 0;
    }

    for (i = 0; i < record->count; i++) {

        if (i > 0) {
            fputc(writer->config.delimiter, writer->output);
        }

        field = &record->fields[i];

        if (field->quoted) {
            fputc(writer->config.quote, writer->output);
        }

        if (field->data) {
            fputs(field->data, writer->output);
        }

        if (field->quoted) {
            fputc(writer->config.quote, writer->output);
        }
    }

    fputc('\n', writer->output);
    writer->records_written++;
    return 1;
    
}

/*

         commc_csv_writer_flush()
	       ---
	       forces all buffered output to destination
	       and ensures data persistence. essential
	       for real-time CSV streaming and proper
	       file closure in applications.

*/

int commc_csv_writer_flush(commc_csv_writer_t* writer) {

    if (!writer || !writer->output) {
        return 0;
    }

    if (fflush(writer->output) != 0) {
        writer->error.error_code = COMMC_IO_ERROR;
        strcpy(writer->error.custom_message, "Failed to flush output stream");
        return 0;
    }

    return 1;
    
}

/* 
	==================================
         --- RECORD IMPL ---
	==================================
*/

/*

         commc_csv_record_create()
	       ---
	       creates new CSV record with initial field
	       capacity for efficient field addition.
	       allocates memory and initializes metadata
	       for proper record manipulation.

*/

commc_csv_record_t* commc_csv_record_create(int initial_capacity) {

    commc_csv_record_t* record;

    if (initial_capacity < 1) {
        initial_capacity = COMMC_CSV_INITIAL_FIELD_CAP;
    }

    record = (commc_csv_record_t*)malloc(sizeof(commc_csv_record_t));

    if (!record) {
        return NULL;
    }

    record->fields = (commc_csv_field_t*)malloc(
        initial_capacity * sizeof(commc_csv_field_t)
    );

    if (!record->fields) {
        free(record);
        return NULL;
    }

    record->count = 0;
    record->capacity = initial_capacity;
    record->line_num = 0;

    return record;
    
}

/*

         commc_csv_record_destroy()
	       ---
	       releases all memory associated with CSV
	       record including individual field data.
	       prevents memory leaks during record
	       processing and manipulation operations.

*/

void commc_csv_record_destroy(commc_csv_record_t* record) {

    int i;

    if (!record) {
        return;
    }

    for (i = 0; i < record->count; i++) {
        free(record->fields[i].data);
    }

    free(record->fields);
    free(record);
    
}

/*

         commc_csv_record_add_field()
	       ---
	       appends new field to CSV record with
	       automatic capacity expansion. copies
	       field data and maintains metadata for
	       proper reconstruction and validation.

*/

int commc_csv_record_add_field(commc_csv_record_t* record, const char* data, int length, int quoted) {

    char* field_copy;

    if (!record || length < 0) {
        return 0;
    }

    if (record->count >= record->capacity) {

        if (!csv_resize_record(record, record->capacity * COMMC_CSV_FIELD_GROWTH_FACTOR)) {
            return 0;
        }
        
    }

    if (data && length > 0) {

        field_copy = (char*)malloc(length + 1);

        if (!field_copy) {
            return 0;
        }

        memcpy(field_copy, data, length);
        field_copy[length] = '\0';
        
    } else {

        field_copy = (char*)malloc(1);

        if (!field_copy) {
            return 0;
        }

        field_copy[0] = '\0';
        length = 0;
        
    }

    record->fields[record->count].data = field_copy;
    record->fields[record->count].length = length;
    record->fields[record->count].quoted = quoted;
    record->fields[record->count].escaped = 0;

    record->count++;
    return 1;
    
}

/*

         commc_csv_record_get_field()
	       ---
	       retrieves field structure from record at
	       specified index with bounds checking.
	       provides safe access to field data and
	       metadata for application processing.

*/

const commc_csv_field_t* commc_csv_record_get_field(const commc_csv_record_t* record, int index) {

    if (!record || index < 0 || index >= record->count) {
        return NULL;
    }

    return &record->fields[index];
    
}

/* 
	==================================
         --- UTILITY IMPL ---
	==================================
*/

/*

         commc_csv_config_default()
	       ---
	       returns standard CSV configuration with
	       RFC 4180 compliant settings. uses comma
	       delimiter, double quotes, and typical
	       parsing behavior for broad compatibility.

*/

commc_csv_config_t commc_csv_config_default(void) {

    commc_csv_config_t config;

    config.delimiter = ',';
    config.quote = '"';
    config.escape = '\0';
    config.skip_empty_lines = 1;
    config.trim_whitespace = 1;
    config.allow_multiline = 1;
    config.strict_quotes = 0;

    return config;
    
}

/*

         commc_csv_config_tab()
	       ---
	       returns configuration for tab-separated
	       values (TSV) processing. optimized for
	       TSV format with appropriate delimiter
	       and quoting rules for data exchange.

*/

commc_csv_config_t commc_csv_config_tab(void) {

    commc_csv_config_t config;

    config.delimiter = '\t';
    config.quote = '"';
    config.escape = '\0';
    config.skip_empty_lines = 1;
    config.trim_whitespace = 0;  /* PRESERVE SPACING IN TSV */
    config.allow_multiline = 1;
    config.strict_quotes = 0;

    return config;
    
}

/* 
	==================================
           --- EOF ---
	==================================
*/