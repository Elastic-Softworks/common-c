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
#define COMMC_CSV_MAX_FIELD_SIZE       1048576

/* 
	==================================
         --- INTERNAL HELPERS ---
	==================================
*/

/*

         csv_is_special_char()
	       ---
	       determines if a character requires special
	       handling in CSV processing. checks for
	       delimiters, quotes, line breaks, and other
	       characters that need escaping or quoting.

*/

static int csv_is_special_char(char c, const commc_csv_config_t* config) {

    return (c == config->delimiter || 
            c == config->quote || 
            c == '\n' || 
            c == '\r' ||
            (config->escape != '\0' && c == config->escape));
            
}

/*

         csv_needs_quoting()
	       ---
	       analyzes field content to determine if quoting
	       is required based on configuration and content.
	       scans for special characters, whitespace, and
	       other conditions requiring quote enclosure.

*/

static int csv_needs_quoting(const char* data, int length, const commc_csv_config_t* config) {

    int i;

    if (config->strict_quotes) {
        return 1;
    }

    for (i = 0; i < length; i++) {

        if (csv_is_special_char(data[i], config)) {
            return 1;
        }
        
    }

    return 0;
    
}

/*

         csv_count_quotes()
	       ---
	       counts quote characters in field data to
	       determine proper escape sequence requirements.
	       essential for calculating output buffer sizes
	       and applying correct quote doubling rules.

*/

static int csv_count_quotes(const char* data, int length, char quote) {

    int count = 0;
    int i;

    for (i = 0; i < length; i++) {

        if (data[i] == quote) {
            count++;
        }
        
    }

    return count;
    
}

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

        char* new_buffer = (char*)commc_realloc(parser->buffer, length);

        if (!new_buffer) {
            commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Failed to resize parser buffer");
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

    commc_error_clear(&parser->error);

    return 1;
    
}

/*

         csv_parser_fill_buffer()
	       ---
	       internal function to read more data from
	       input file into parser buffer. handles
	       end-of-file conditions and maintains
	       efficient streaming for large CSV files.

*/

static int csv_parser_fill_buffer(commc_csv_parser_t* parser) {

    int bytes_read;

    if (!parser->input) {
        return 0;  /* STRING MODE - NO REFILL NEEDED */
    }

    if (feof(parser->input)) {
        return 0;
    }

    bytes_read = fread(parser->buffer, 1, parser->buf_size, parser->input);

    if (bytes_read <= 0) {
        return 0;
    }

    parser->buf_len = bytes_read;
    parser->buf_pos = 0;

    return 1;
    
}

/*

         csv_parser_get_char()
	       ---
	       retrieves next character from input stream
	       with buffer management. handles line and
	       column tracking for error reporting and
	       automatically refills buffer when needed.

*/

static int csv_parser_get_char(commc_csv_parser_t* parser) {

    char c;

    if (parser->buf_pos >= parser->buf_len) {

        if (!csv_parser_fill_buffer(parser)) {
            return -1;  /* END OF INPUT */
        }
        
    }

    c = parser->buffer[parser->buf_pos++];

    if (c == '\n') {
        parser->line_num++;
        parser->col_num = 1;
    } else {
        parser->col_num++;
    }

    return (int)(unsigned char)c;
    
}

/*

         csv_parser_peek_char()
	       ---
	       examines next character without consuming it
	       from input stream. essential for lookahead
	       parsing decisions and quote sequence detection
	       in CSV field processing algorithms.

*/

static int csv_parser_peek_char(commc_csv_parser_t* parser) {

    if (parser->buf_pos >= parser->buf_len) {

        if (!csv_parser_fill_buffer(parser)) {
            return -1;  /* END OF INPUT */
        }
        
    }

    return (int)(unsigned char)parser->buffer[parser->buf_pos];
    
}

/*

         csv_parser_skip_whitespace()
	       ---
	       advances past whitespace characters when
	       trim_whitespace configuration is enabled.
	       handles space and tab characters while
	       preserving line position tracking.

*/

static void csv_parser_skip_whitespace(commc_csv_parser_t* parser) {

    int c;

    if (!parser->config.trim_whitespace) {
        return;
    }

    while ((c = csv_parser_peek_char(parser)) != -1) {

        if (c != ' ' && c != '\t') {
            break;
        }

        csv_parser_get_char(parser);
        
    }
    
}

/*

         csv_parser_parse_field()
	       ---
	       parses a single CSV field with proper quote
	       handling, escape sequence processing, and
	       multiline support. builds field content
	       with appropriate metadata for reconstruction.

*/

static char* csv_parser_parse_field(commc_csv_parser_t* parser, int* field_length, int* quoted) {

    char* field_data = NULL;
    int   capacity = 256;
    int   length = 0;
    int   c;
    int   in_quotes = 0;
    int   field_quoted = 0;

    field_data = (char*)commc_malloc(capacity);

    if (!field_data) {
        commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Failed to allocate field buffer");
        return NULL;
    }

    csv_parser_skip_whitespace(parser);

    c = csv_parser_peek_char(parser);

    if (c == parser->config.quote) {
        field_quoted = 1;
        in_quotes = 1;
        csv_parser_get_char(parser);  /* CONSUME OPENING QUOTE */
    }

    while ((c = csv_parser_get_char(parser)) != -1) {

        if (in_quotes) {

            if (c == parser->config.quote) {

                int next_c = csv_parser_peek_char(parser);

                if (next_c == parser->config.quote) {

                    /* ESCAPED QUOTE - ADD SINGLE QUOTE TO OUTPUT */
                    csv_parser_get_char(parser);  /* CONSUME SECOND QUOTE */
                    
                    if (length >= capacity - 1) {
                        capacity *= 2;
                        field_data = (char*)commc_realloc(field_data, capacity);
                        if (!field_data) {
                            commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Field too large");
                            return NULL;
                        }
                    }
                    
                    field_data[length++] = parser->config.quote;
                    
                } else {

                    /* END OF QUOTED FIELD */
                    in_quotes = 0;
                    
                }
                
            } else if (c == '\n' && !parser->config.allow_multiline) {

                commc_error_set(&parser->error, COMMC_ERROR_PARSE, "Unexpected newline in quoted field");
                commc_free(field_data);
                return NULL;
                
            } else {

                /* REGULAR CHARACTER IN QUOTED FIELD */
                if (length >= capacity - 1) {
                    capacity *= 2;
                    field_data = (char*)commc_realloc(field_data, capacity);
                    if (!field_data) {
                        commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Field too large");
                        return NULL;
                    }
                }
                
                field_data[length++] = c;
                
            }
            
        } else {

            /* NOT IN QUOTES */
            if (c == parser->config.delimiter || c == '\n' || c == '\r') {

                /* END OF FIELD */
                break;
                
            } else if (c == parser->config.quote && length == 0) {

                /* DELAYED QUOTE START */
                field_quoted = 1;
                in_quotes = 1;
                
            } else {

                /* REGULAR UNQUOTED CHARACTER */
                if (length >= capacity - 1) {
                    capacity *= 2;
                    field_data = (char*)commc_realloc(field_data, capacity);
                    if (!field_data) {
                        commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Field too large");
                        return NULL;
                    }
                }
                
                field_data[length++] = c;
                
            }
            
        }
        
    }

    if (in_quotes) {
        commc_error_set(&parser->error, COMMC_ERROR_PARSE, "Unterminated quoted field");
        commc_free(field_data);
        return NULL;
    }

    if (parser->config.trim_whitespace && !field_quoted) {

        /* TRIM TRAILING WHITESPACE FROM UNQUOTED FIELDS */
        while (length > 0 && (field_data[length-1] == ' ' || field_data[length-1] == '\t')) {
            length--;
        }
        
    }

    field_data[length] = '\0';
    *field_length = length;
    *quoted = field_quoted;

    return field_data;
    
}

/*

         commc_csv_parser_next_record()
	       ---
	       reads and parses complete CSV record from
	       input stream. handles field parsing, record
	       assembly, and error tracking with detailed
	       context for troubleshooting parsing issues.

*/

commc_csv_record_t* commc_csv_parser_next_record(commc_csv_parser_t* parser) {

    commc_csv_record_t* record;
    char*               field_data;
    int                 field_length;
    int                 field_quoted;
    int                 c;
    int                 record_line;

    if (!parser) {
        return NULL;
    }

    /* SKIP EMPTY LINES IF CONFIGURED */
    while (parser->config.skip_empty_lines) {

        c = csv_parser_peek_char(parser);

        if (c == -1) {
            return NULL;  /* END OF INPUT */
        }

        if (c != '\n' && c != '\r') {
            break;  /* NON-EMPTY LINE FOUND */
        }

        csv_parser_get_char(parser);  /* CONSUME EMPTY LINE */
        
    }

    record_line = parser->line_num;

    record = commc_csv_record_create(COMMC_CSV_INITIAL_FIELD_CAP);

    if (!record) {
        commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Failed to create CSV record");
        return NULL;
    }

    record->line_num = record_line;

    /* PARSE FIELDS UNTIL END OF RECORD */
    while (1) {

        field_data = csv_parser_parse_field(parser, &field_length, &field_quoted);

        if (!field_data) {

            if (commc_error_is_set(&parser->error)) {
                commc_csv_record_destroy(record);
                return NULL;
            }

            /* END OF INPUT - RETURN PARTIAL RECORD IF IT HAS FIELDS */
            if (record->count > 0) {
                return record;
            } else {
                commc_csv_record_destroy(record);
                return NULL;
            }
            
        }

        if (!commc_csv_record_add_field(record, field_data, field_length, field_quoted)) {
            commc_error_set(&parser->error, COMMC_ERROR_MEMORY, "Failed to add field to record");
            commc_free(field_data);
            commc_csv_record_destroy(record);
            return NULL;
        }

        commc_free(field_data);

        c = csv_parser_peek_char(parser);

        if (c == parser->config.delimiter) {

            csv_parser_get_char(parser);  /* CONSUME DELIMITER */
            continue;  /* MORE FIELDS IN THIS RECORD */
            
        } else if (c == '\n') {

            csv_parser_get_char(parser);  /* CONSUME NEWLINE */
            break;  /* END OF RECORD */
            
        } else if (c == '\r') {

            csv_parser_get_char(parser);  /* CONSUME CR */
            
            if (csv_parser_peek_char(parser) == '\n') {
                csv_parser_get_char(parser);  /* CONSUME LF */
            }
            
            break;  /* END OF RECORD */
            
        } else if (c == -1) {

            break;  /* END OF INPUT */
            
        } else {

            commc_error_set(&parser->error, COMMC_ERROR_PARSE, "Unexpected character after field");
            commc_csv_record_destroy(record);
            return NULL;
            
        }
        
    }

    return record;
    
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

    writer = (commc_csv_writer_t*)commc_malloc(sizeof(commc_csv_writer_t));

    if (!writer) {
        return NULL;
    }

    memcpy(&writer->config, config, sizeof(commc_csv_config_t));

    writer->buffer = (char*)commc_malloc(buffer_size);

    if (!writer->buffer) {
        commc_free(writer);
        return NULL;
    }

    writer->buf_size = buffer_size;
    writer->buf_pos = 0;
    writer->output = NULL;
    writer->records_written = 0;

    commc_error_clear(&writer->error);

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

    commc_free(writer->buffer);
    commc_free(writer);
    
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
        commc_error_set(&writer->error, COMMC_ERROR_IO, "Failed to create CSV output file");
        return 0;
    }

    writer->buf_pos = 0;
    writer->records_written = 0;

    commc_error_clear(&writer->error);

    return 1;
    
}

/*

         csv_writer_flush_buffer()
	       ---
	       internal function to write buffered data
	       to output destination. handles file I/O
	       errors and maintains efficient output
	       streaming for large CSV generation.

*/

static int csv_writer_flush_buffer(commc_csv_writer_t* writer) {

    int bytes_written;

    if (writer->buf_pos == 0) {
        return 1;  /* NOTHING TO FLUSH */
    }

    if (writer->output) {

        bytes_written = fwrite(writer->buffer, 1, writer->buf_pos, writer->output);

        if (bytes_written != writer->buf_pos) {
            commc_error_set(&writer->error, COMMC_ERROR_IO, "Failed to write CSV data");
            return 0;
        }
        
    }

    writer->buf_pos = 0;
    return 1;
    
}

/*

         csv_writer_write_char()
	       ---
	       writes single character to output buffer
	       with automatic flushing when buffer fills.
	       maintains efficient character-by-character
	       output for CSV formatting operations.

*/

static int csv_writer_write_char(commc_csv_writer_t* writer, char c) {

    if (writer->buf_pos >= writer->buf_size) {

        if (!csv_writer_flush_buffer(writer)) {
            return 0;
        }
        
    }

    writer->buffer[writer->buf_pos++] = c;
    return 1;
    
}

/*

         csv_writer_write_string()
	       ---
	       writes string data to output buffer with
	       automatic buffer management. handles string
	       length efficiently and flushes when needed
	       to maintain consistent output streaming.

*/

static int csv_writer_write_string(commc_csv_writer_t* writer, const char* data, int length) {

    int i;

    for (i = 0; i < length; i++) {

        if (!csv_writer_write_char(writer, data[i])) {
            return 0;
        }
        
    }

    return 1;
    
}

/*

         commc_csv_writer_write_record()
	       ---
	       writes complete CSV record with proper
	       field formatting, escaping, and delimiter
	       placement. handles quote insertion and
	       escape sequence generation automatically.

*/

int commc_csv_writer_write_record(commc_csv_writer_t* writer, const commc_csv_record_t* record) {

    int i;
    const commc_csv_field_t* field;

    if (!writer || !record) {
        return 0;
    }

    for (i = 0; i < record->count; i++) {

        if (i > 0) {

            if (!csv_writer_write_char(writer, writer->config.delimiter)) {
                return 0;
            }
            
        }

        field = &record->fields[i];

        if (field->quoted || csv_needs_quoting(field->data, field->length, &writer->config)) {

            if (!csv_writer_write_char(writer, writer->config.quote)) {
                return 0;
            }

            /* WRITE FIELD DATA WITH QUOTE ESCAPING */
            if (field->data) {

                int j;

                for (j = 0; j < field->length; j++) {

                    if (field->data[j] == writer->config.quote) {

                        /* DOUBLE THE QUOTE FOR ESCAPING */
                        if (!csv_writer_write_char(writer, writer->config.quote) ||
                            !csv_writer_write_char(writer, writer->config.quote)) {
                            return 0;
                        }
                        
                    } else {

                        if (!csv_writer_write_char(writer, field->data[j])) {
                            return 0;
                        }
                        
                    }
                    
                }
                
            }

            if (!csv_writer_write_char(writer, writer->config.quote)) {
                return 0;
            }
            
        } else {

            /* UNQUOTED FIELD */
            if (field->data && !csv_writer_write_string(writer, field->data, field->length)) {
                return 0;
            }
            
        }
        
    }

    if (!csv_writer_write_char(writer, '\n')) {
        return 0;
    }

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

    if (!writer) {
        return 0;
    }

    if (!csv_writer_flush_buffer(writer)) {
        return 0;
    }

    if (writer->output && fflush(writer->output) != 0) {
        commc_error_set(&writer->error, COMMC_ERROR_IO, "Failed to flush output stream");
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

    record = (commc_csv_record_t*)commc_malloc(sizeof(commc_csv_record_t));

    if (!record) {
        return NULL;
    }

    record->fields = (commc_csv_field_t*)commc_malloc(
        initial_capacity * sizeof(commc_csv_field_t)
    );

    if (!record->fields) {
        commc_free(record);
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
        commc_free(record->fields[i].data);
    }

    commc_free(record->fields);
    commc_free(record);
    
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

        field_copy = (char*)commc_malloc(length + 1);

        if (!field_copy) {
            return 0;
        }

        memcpy(field_copy, data, length);
        field_copy[length] = '\0';
        
    } else {

        field_copy = (char*)commc_malloc(1);

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