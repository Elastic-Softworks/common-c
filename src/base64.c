/*

   ===================================
   C O M M C   -   B A S E 6 4
   BASE64 ENCODING AND DECODING
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#include "commc/base64.h"
#include <stdlib.h>
#include <string.h>

/* 
	==================================
          --- CONSTANTS ---
	==================================
*/

/* standard Base64 alphabet for encoding binary data */

static const char BASE64_STANDARD[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* URL-safe Base64 alphabet with - and _ instead of + and / */

static const char BASE64_URL_SAFE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/* decode table for standard Base64 characters (invalid = 255) */

static const unsigned char DECODE_TABLE_STANDARD[256] = {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,
    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

/* decode table for URL-safe Base64 characters */

static const unsigned char DECODE_TABLE_URL_SAFE[256] = {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255, 62,255,255,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,
    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

/* 
	==================================
         --- ENCODER API ---
	==================================
*/

/*

         commc_base64_encoder_create()
	       ---
	       allocates and initializes a streaming Base64
	       encoder with the specified configuration and
	       output buffer capacity for incremental
	       encoding of binary data streams.

*/

commc_base64_encoder_t* commc_base64_encoder_create(const commc_base64_config_t* config, int output_capacity) {

    commc_base64_encoder_t* encoder;

    if (!config || output_capacity < 4) {
        return NULL;
    }

    encoder = (commc_base64_encoder_t*)malloc(sizeof(commc_base64_encoder_t));

    if (!encoder) {
        return NULL;
    }

    encoder->config   = *config;
    encoder->buf_pos  = 0;
    encoder->out_size = output_capacity;
    encoder->out_pos  = 0;
    encoder->line_pos = 0;
    encoder->total_in = 0;
    encoder->total_out = 0;

    encoder->output = (char*)malloc(output_capacity);

    if (!encoder->output) {
        free(encoder);
        return NULL;
    }

    /* initialize error context */

    memset(&encoder->error, 0, sizeof(commc_error_context_t));

    return encoder;
}

/*

         commc_base64_encoder_destroy()
	       ---
	       releases all memory and resources associated
	       with the Base64 encoder including output
	       buffers and state information to prevent
	       memory leaks during cleanup.

*/

void commc_base64_encoder_destroy(commc_base64_encoder_t* encoder) {

    if (!encoder) {
        return;
    }

    if (encoder->output) {
        free(encoder->output);
    }

    free(encoder);
}

/*

         encoder_grow_output()
	       ---
	       internal helper function to expand encoder
	       output buffer when current capacity is
	       insufficient. doubles buffer size to
	       accommodate growing encoded data.

*/

static int encoder_grow_output(commc_base64_encoder_t* encoder, int min_size) {

    int   new_size;
    char* new_buffer;

    new_size = encoder->out_size * 2;

    if (new_size < min_size) {

        new_size = min_size + 1024;

    }

    new_buffer = (char*)realloc(encoder->output, new_size);

    if (!new_buffer) {

        encoder->error.error_code = COMMC_MEMORY_ERROR;
        strcpy(encoder->error.custom_message, "Failed to grow output buffer");
        return -1;

    }

    encoder->output = new_buffer;
    encoder->out_size = new_size;

    return 0;
}

/*

         encoder_write_chars()
	       ---
	       internal helper to write encoded characters
	       to output buffer with line wrapping and
	       buffer growth management. handles MIME
	       line length restrictions automatically.

*/

static int encoder_write_chars(commc_base64_encoder_t* encoder, const char* chars, int count) {

    int i;

    for (i = 0; i < count; i++) {

        /* check if we need line wrapping */

        if (encoder->config.line_wrap && encoder->config.wrap_length > 0) {

            if (encoder->line_pos >= encoder->config.wrap_length) {

                int ending_len = strlen(encoder->config.line_ending);

                /* ensure output buffer has space for line ending */

                if (encoder->out_pos + ending_len >= encoder->out_size) {

                    if (encoder_grow_output(encoder, encoder->out_pos + ending_len) < 0) {
                        return -1;
                    }
                }

                memcpy(&encoder->output[encoder->out_pos], encoder->config.line_ending, ending_len);
                encoder->out_pos += ending_len;
                encoder->line_pos = 0;
            }
        }

        /* ensure buffer space for character */

        if (encoder->out_pos + 1 >= encoder->out_size) {

            if (encoder_grow_output(encoder, encoder->out_pos + 1) < 0) {
                return -1;
            }
        }

        encoder->output[encoder->out_pos++] = chars[i];
        encoder->line_pos++;
    }

    return 0;
}

/*

         commc_base64_encoder_feed()
	       ---
	       feeds binary data to encoder for incremental
	       processing. manages byte triplet buffering
	       and generates Base64 output characters as
	       complete groups become available.

*/

int commc_base64_encoder_feed(commc_base64_encoder_t* encoder, const unsigned char* data, int length) {

    const char* alphabet;
    int         i;
    char        output_chars[4];

    if (!encoder || !data || length < 0) {

        return -1;

    }

    alphabet = encoder->config.url_safe ? BASE64_URL_SAFE : BASE64_STANDARD;

    for (i = 0; i < length; i++) {

        encoder->buffer[encoder->buf_pos++] = data[i];
        encoder->total_in++;

        /* process complete triplet */

        if (encoder->buf_pos == 3) {

            unsigned int triplet = (encoder->buffer[0] << 16) |
                                   (encoder->buffer[1] << 8)  |
                                    encoder->buffer[2];

            output_chars[0] = alphabet[(triplet >> 18) & 0x3F];
            output_chars[1] = alphabet[(triplet >> 12) & 0x3F];
            output_chars[2] = alphabet[(triplet >> 6)  & 0x3F];
            output_chars[3] = alphabet[triplet         & 0x3F];

            if (encoder_write_chars(encoder, output_chars, 4) < 0) {
                return -1;
            }

            encoder->total_out += 4;
            encoder->buf_pos = 0;
        }
    }

    return 0;
}

/*

         commc_base64_encoder_finish()
	       ---
	       completes encoding process by handling final
	       partial byte group with appropriate padding.
	       generates remaining output and finalizes
	       encoded string with null termination.

*/

int commc_base64_encoder_finish(commc_base64_encoder_t* encoder) {

    const char*  alphabet;
    char         output_chars[4];
    unsigned int triplet;

    if (!encoder) {
        return -1;
    }

    alphabet = encoder->config.url_safe ? BASE64_URL_SAFE : BASE64_STANDARD;

    /* handle partial triplet with padding */

    if (encoder->buf_pos > 0) {

        /* pad buffer with zeros */

        while (encoder->buf_pos < 3) {

            encoder->buffer[encoder->buf_pos++] = 0;

        }

        triplet = (encoder->buffer[0] << 16) |
                  (encoder->buffer[1] << 8)  |
                   encoder->buffer[2];

        output_chars[0] = alphabet[(triplet >> 18) & 0x3F];
        output_chars[1] = alphabet[(triplet >> 12) & 0x3F];

        if (encoder->total_in % 3 == 1) {

            /* one input byte: two output chars + padding */

            if (encoder->config.use_padding) {

                output_chars[2] = '=';
                output_chars[3] = '=';
                if (encoder_write_chars(encoder, output_chars, 4) < 0) {

                    return -1;

                }
                encoder->total_out += 4;

            } else {

                if (encoder_write_chars(encoder, output_chars, 2) < 0) {

                    return -1;

                }

                encoder->total_out += 2;
            }

        } else if (encoder->total_in % 3 == 2) {

            /* two input bytes: three output chars + padding */

            output_chars[2] = alphabet[(triplet >> 6) & 0x3F];

            if (encoder->config.use_padding) {

                output_chars[3] = '=';

                if (encoder_write_chars(encoder, output_chars, 4) < 0) {

                    return -1;

                }

                encoder->total_out += 4;

            } else {

                if (encoder_write_chars(encoder, output_chars, 3) < 0) {

                    return -1;

                }

                encoder->total_out += 3;
            }
        }
    }

    /* null terminate output string */

    if (encoder->out_pos >= encoder->out_size) {

        if (encoder_grow_output(encoder, encoder->out_pos + 1) < 0) {
            return -1;
        }
    }

    encoder->output[encoder->out_pos] = '\0';

    return 0;
}

/*

         commc_base64_encoder_get_output()
	       ---
	       returns pointer to current encoded output
	       string. the string is null-terminated and
	       remains valid until encoder is destroyed
	       or reset for reuse.

*/

const char* commc_base64_encoder_get_output(const commc_base64_encoder_t* encoder) {

    if (!encoder || !encoder->output) {
        return NULL;
    }

    return encoder->output;
}

/*

         commc_base64_encoder_reset()
	       ---
	       resets encoder state for reuse with new
	       data while preserving configuration and
	       output buffer allocation for efficient
	       encoder recycling and reuse.

*/

void commc_base64_encoder_reset(commc_base64_encoder_t* encoder) {

    if (!encoder) {
        return;
    }

    encoder->buf_pos   = 0;
    encoder->out_pos   = 0;
    encoder->line_pos  = 0;
    encoder->total_in  = 0;
    encoder->total_out = 0;

    memset(&encoder->error, 0, sizeof(commc_error_context_t));
}

/* 
	==================================
         --- DECODER API ---
	==================================
*/

/*

         commc_base64_decoder_create()
	       ---
	       allocates and initializes a streaming Base64
	       decoder with the specified configuration and
	       output buffer capacity for incremental
	       decoding of Base64 strings.

*/

commc_base64_decoder_t* commc_base64_decoder_create(const commc_base64_config_t* config, int output_capacity) {

    commc_base64_decoder_t* decoder;

    if (!config || output_capacity < 3) {
        return NULL;
    }

    decoder = (commc_base64_decoder_t*)malloc(sizeof(commc_base64_decoder_t));

    if (!decoder) {
        return NULL;
    }

    decoder->config        = *config;
    decoder->buf_pos       = 0;
    decoder->out_size      = output_capacity;
    decoder->out_pos       = 0;
    decoder->padding_count = 0;
    decoder->finished      = 0;
    decoder->total_in      = 0;
    decoder->total_out     = 0;

    decoder->output = (unsigned char*)malloc(output_capacity);

    if (!decoder->output) {
        free(decoder);
        return NULL;
    }

    /* initialize error context */
    memset(&decoder->error, 0, sizeof(commc_error_context_t));

    return decoder;
}

/*

         commc_base64_decoder_destroy()
	       ---
	       releases all memory and resources associated
	       with the Base64 decoder including output
	       buffers and state information to prevent
	       memory leaks during cleanup.

*/

void commc_base64_decoder_destroy(commc_base64_decoder_t* decoder) {

    if (!decoder) {
        return;
    }

    if (decoder->output) {
        free(decoder->output);
    }

    free(decoder);
}

/*

         decoder_grow_output()
	       ---
	       internal helper function to expand decoder
	       output buffer when current capacity is
	       insufficient. doubles buffer size to
	       accommodate growing decoded data.

*/

static int decoder_grow_output(commc_base64_decoder_t* decoder, int min_size) {

    int            new_size;
    unsigned char* new_buffer;

    new_size = decoder->out_size * 2;

    if (new_size < min_size) {
        new_size = min_size + 1024;
    }

    new_buffer = (unsigned char*)realloc(decoder->output, new_size);

    if (!new_buffer) {
        decoder->error.error_code = COMMC_MEMORY_ERROR;
        strcpy(decoder->error.custom_message, "Failed to grow output buffer");
        return -1;
    }

    decoder->output = new_buffer;
    decoder->out_size = new_size;

    return 0;
}

/*

         commc_base64_decoder_feed()
	       ---
	       feeds Base64 characters to decoder for
	       incremental processing. handles character
	       validation, quartet buffering, and binary
	       output generation as complete groups decode.

*/

int commc_base64_decoder_feed(commc_base64_decoder_t* decoder, const char* data, int length) {

    const unsigned char* decode_table;
    int                  i;
    unsigned char        decoded_value;

    if (!decoder || !data || length < 0 || decoder->finished) {
        return -1;
    }

    decode_table = decoder->config.url_safe ? DECODE_TABLE_URL_SAFE : DECODE_TABLE_STANDARD;

    for (i = 0; i < length; i++) {

        char ch = data[i];

        /* skip whitespace and line endings */

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            continue;
        }

        decoder->total_in++;

        /* handle padding characters */

        if (ch == '=') {

            if (!decoder->config.use_padding) {

                if (decoder->config.strict_decode) {
                    decoder->error.error_code = COMMC_FORMAT_ERROR;
                    strcpy(decoder->error.custom_message, "Unexpected padding in no-padding mode");
                    return -1;
                }
                continue;
            }

            decoder->padding_count++;

            if (decoder->padding_count > 2) {

                if (decoder->config.strict_decode) {
                    decoder->error.error_code = COMMC_FORMAT_ERROR;
                    strcpy(decoder->error.custom_message, "Too many padding characters");
                    return -1;
                }
                continue;
            }

            /* padding marks end of data */
            decoder->finished = 1;
            continue;
        }

        /* validate character */

        decoded_value = decode_table[(unsigned char)ch];

        if (decoded_value == 255) {

            if (decoder->config.strict_decode) {
                decoder->error.error_code = COMMC_FORMAT_ERROR;
                strcpy(decoder->error.custom_message, "Invalid Base64 character");
                return -1;
            }
            continue;
        }

        /* no more input allowed after padding */

        if (decoder->finished) {

            if (decoder->config.strict_decode) {
                decoder->error.error_code = COMMC_FORMAT_ERROR;
                strcpy(decoder->error.custom_message, "Data after padding characters");
                return -1;
            }
            continue;
        }

        decoder->buffer[decoder->buf_pos++] = decoded_value;

        /* process complete quartet */

        if (decoder->buf_pos == 4) {

            unsigned int quartet = (decoder->buffer[0] << 18) |
                                   (decoder->buffer[1] << 12) |
                                   (decoder->buffer[2] << 6)  |
                                    decoder->buffer[3];

            /* ensure output buffer space */

            if (decoder->out_pos + 3 >= decoder->out_size) {

                if (decoder_grow_output(decoder, decoder->out_pos + 3) < 0) {
                    return -1;
                }
            }

            decoder->output[decoder->out_pos++] = (quartet >> 16) & 0xFF;
            decoder->output[decoder->out_pos++] = (quartet >> 8)  & 0xFF;
            decoder->output[decoder->out_pos++] = quartet         & 0xFF;

            decoder->total_out += 3;
            decoder->buf_pos = 0;
        }
    }

    return 0;
}

/*

         commc_base64_decoder_finish()
	       ---
	       completes decoding process by handling final
	       character group and validating padding.
	       generates remaining binary output and
	       performs comprehensive validation checks.

*/

int commc_base64_decoder_finish(commc_base64_decoder_t* decoder) {

    unsigned int quartet;
    int          valid_bytes;

    if (!decoder) {
        return -1;
    }

    /* handle partial quartet */

    if (decoder->buf_pos > 0) {

        if (decoder->buf_pos < 2) {

            if (decoder->config.strict_decode) {
                decoder->error.error_code = COMMC_FORMAT_ERROR;
                strcpy(decoder->error.custom_message, "Incomplete Base64 input");
                return -1;
            }
            return 0;
        }

        /* pad buffer with zeros for decoding */
        while (decoder->buf_pos < 4) {
            decoder->buffer[decoder->buf_pos++] = 0;
        }

        quartet = (decoder->buffer[0] << 18) |
                  (decoder->buffer[1] << 12) |
                  (decoder->buffer[2] << 6)  |
                   decoder->buffer[3];

        /* determine valid output bytes based on padding */

        if (decoder->padding_count == 2) {
            valid_bytes = 1;  /* == padding */
        } else if (decoder->padding_count == 1) {
            valid_bytes = 2;  /* = padding */
        } else {
            valid_bytes = 3;  /* no padding */
        }

        /* ensure output buffer space */

        if (decoder->out_pos + valid_bytes >= decoder->out_size) {

            if (decoder_grow_output(decoder, decoder->out_pos + valid_bytes) < 0) {
                return -1;
            }
        }

        if (valid_bytes >= 1) {
            decoder->output[decoder->out_pos++] = (quartet >> 16) & 0xFF;
            decoder->total_out++;
        }

        if (valid_bytes >= 2) {
            decoder->output[decoder->out_pos++] = (quartet >> 8) & 0xFF;
            decoder->total_out++;
        }

        if (valid_bytes >= 3) {
            decoder->output[decoder->out_pos++] = quartet & 0xFF;
            decoder->total_out++;
        }
    }

    decoder->finished = 1;

    return 0;
}

/*

         commc_base64_decoder_get_output()
	       ---
	       returns pointer to current decoded binary
	       output with size information. the data
	       remains valid until decoder is destroyed
	       or reset for reuse.

*/

const unsigned char* commc_base64_decoder_get_output(const commc_base64_decoder_t* decoder, int* size) {

    if (!decoder || !size) {

        return NULL;

    }

    *size = decoder->out_pos;

    return decoder->output;
}

/*

         commc_base64_decoder_reset()
	       ---
	       resets decoder state for reuse with new
	       data while preserving configuration and
	       output buffer allocation for efficient
	       decoder recycling and reuse.

*/

void commc_base64_decoder_reset(commc_base64_decoder_t* decoder) {

    if (!decoder) {

        return;

    }

    decoder->buf_pos       = 0;
    decoder->out_pos       = 0;
    decoder->padding_count = 0;
    decoder->finished      = 0;
    decoder->total_in      = 0;
    decoder->total_out     = 0;

    memset(&decoder->error, 0, sizeof(commc_error_context_t));
}

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_base64_encode()
	       ---
	       convenient wrapper function for simple Base64
	       encoding in a single operation without
	       streaming requirements or complex state
	       management. returns allocated string.

*/

char* commc_base64_encode(const unsigned char* data, int length, const commc_base64_config_t* config) {

    commc_base64_encoder_t* encoder;
    const char*             output;
    char*                   result;
    int                     output_size;

    if (!data || length < 0 || !config) {

        return NULL;

    }

    output_size = commc_base64_get_encoded_size(length, config);
    encoder = commc_base64_encoder_create(config, output_size + 128);

    if (!encoder) {

        return NULL;

    }

    if (commc_base64_encoder_feed(encoder, data, length) < 0) {

        commc_base64_encoder_destroy(encoder);
        return NULL;

    }

    if (commc_base64_encoder_finish(encoder) < 0) {

        commc_base64_encoder_destroy(encoder);
        return NULL;

    }

    output = commc_base64_encoder_get_output(encoder);

    if (!output) {

        commc_base64_encoder_destroy(encoder);
        return NULL;

    }

    result = (char*)malloc(strlen(output) + 1);

    if (result) {

        strcpy(result, output);

    }

    commc_base64_encoder_destroy(encoder);

    return result;
}

/*

         commc_base64_decode()
	       ---
	       convenient wrapper function for simple Base64
	       decoding in a single operation without
	       streaming requirements or complex state
	       management. returns allocated result structure.

*/

commc_base64_result_t* commc_base64_decode(const char* data, int length, const commc_base64_config_t* config) {

    commc_base64_decoder_t* decoder;
    commc_base64_result_t*  result;
    const unsigned char*    output;
    int                     output_size;
    int                     decoded_size;

    if (!data || length < 0 || !config) {

        return NULL;

    }

    if (length == 0) {

        length = strlen(data);

    }

    decoded_size = commc_base64_get_decoded_size(length, config);
    decoder = commc_base64_decoder_create(config, decoded_size + 64);

    if (!decoder) {

        return NULL;

    }

    if (commc_base64_decoder_feed(decoder, data, length) < 0) {

        commc_base64_decoder_destroy(decoder);
        return NULL;

    }

    if (commc_base64_decoder_finish(decoder) < 0) {

        commc_base64_decoder_destroy(decoder);
        return NULL;

    }

    output = commc_base64_decoder_get_output(decoder, &output_size);

    if (!output && output_size > 0) {

        commc_base64_decoder_destroy(decoder);
        return NULL;

    }

    result = (commc_base64_result_t*)malloc(sizeof(commc_base64_result_t));

    if (!result) {

        commc_base64_decoder_destroy(decoder);
        return NULL;

    }

    result->size           = output_size;
    result->capacity       = output_size;
    result->input_consumed = decoder->total_in;
    result->is_complete    = decoder->finished;

    if (output_size > 0) {

        result->data = (unsigned char*)malloc(output_size);

        if (!result->data) {

            free(result);
            commc_base64_decoder_destroy(decoder);
            return NULL;

        }

        memcpy(result->data, output, output_size);

    } else {

        result->data = NULL;

    }

    commc_base64_decoder_destroy(decoder);

    return result;
}

/*

         commc_base64_get_encoded_size()
	       ---
	       calculates maximum encoded output size for
	       given input length considering padding and
	       line wrapping requirements. useful for
	       buffer allocation planning.

*/

int commc_base64_get_encoded_size(int input_length, const commc_base64_config_t* config) {

    int encoded_chars;
    int lines;
    int line_endings;

    if (input_length < 0 || !config) {

        return 0;

    }

    /* calculate base64 characters (4 chars per 3 input bytes) */

    if (config->use_padding) {

        encoded_chars = ((input_length + 2) / 3) * 4;

    } else {

        encoded_chars = (input_length * 4 + 2) / 3;

    }

    /* add line wrapping overhead */

    if (config->line_wrap && config->wrap_length > 0) {

        lines = (encoded_chars + config->wrap_length - 1) / config->wrap_length;
        line_endings = (lines - 1) * strlen(config->line_ending);

        encoded_chars += line_endings;
    }

    return encoded_chars + 1;  /* +1 for null terminator */
}

/*

         commc_base64_get_decoded_size()
	       ---
	       estimates maximum decoded output size for
	       Base64 input considering padding. provides
	       upper bound for buffer allocation during
	       decoding operations.

*/

int commc_base64_get_decoded_size(int input_length, const commc_base64_config_t* config) {

    int decoded_bytes;

    if (input_length < 0 || !config) {

        return 0;

    }

    /* estimate decoded size (3 bytes per 4 input chars) */

    decoded_bytes = (input_length * 3) / 4;

    return decoded_bytes;
}

/*

         commc_base64_validate()
	       ---
	       validates Base64 string format according to
	       configuration requirements. checks character
	       validity, padding correctness, and overall
	       format compliance before decoding.

*/

int commc_base64_validate(const char* data, int length, const commc_base64_config_t* config) {

    const unsigned char* decode_table;
    int                  i;
    int                  padding_count;
    int                  content_chars;
    int                  finished;

    if (!data || !config) {

        return 0;

    }

    if (length == 0) {

        length = strlen(data);

    }

    decode_table   = config->url_safe ? DECODE_TABLE_URL_SAFE : DECODE_TABLE_STANDARD;
    padding_count  = 0;
    content_chars  = 0;
    finished       = 0;

    for (i = 0; i < length; i++) {

        char ch = data[i];

        /* skip whitespace */

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {

            continue;

        }

        /* handle padding */

        if (ch == '=') {

            if (!config->use_padding) {

                return 0;  /* unexpected padding */

            }

            padding_count++;

            if (padding_count > 2) {

                return 0;  /* too much padding */

            }

            finished = 1;
            continue;
        }

        /* no content after padding */

        if (finished) {

            return 0;

        }

        /* validate character */

        if (decode_table[(unsigned char)ch] == 255) {

            return 0;  /* invalid character */

        }

        content_chars++;

    }

    /* check minimum length */

    if (content_chars == 0) {

        return 1;  /* empty string is valid */

    }

    if (config->use_padding) {

        /* with padding: total must be multiple of 4 */

        return ((content_chars + padding_count) % 4) == 0;

    } else {

        /* without padding: content must be at least 2 chars */

        return content_chars >= 2;

    }
}

/*

         commc_base64_config_default()
	       ---
	       returns standard Base64 configuration with
	       padding enabled, standard alphabet, and no
	       line wrapping for general purpose encoding
	       and decoding operations.

*/

commc_base64_config_t commc_base64_config_default(void) {

    commc_base64_config_t config;

    config.use_padding   = 1;
    config.line_wrap     = 0;
    config.wrap_length   = 0;
    config.url_safe      = 0;
    config.strict_decode = 1;

    strcpy(config.line_ending, "\n");

    return config;
}

/*

         commc_base64_config_url_safe()
	       ---
	       returns URL-safe Base64 configuration using
	       modified alphabet with - and _ characters
	       instead of + and /, typically without
	       padding for URL and filename compatibility.

*/

commc_base64_config_t commc_base64_config_url_safe(void) {

    commc_base64_config_t config;

    config.use_padding   = 0;  /* URL-safe typically omits padding */
    config.line_wrap     = 0;
    config.wrap_length   = 0;
    config.url_safe      = 1;
    config.strict_decode = 1;

    strcpy(config.line_ending, "\n");

    return config;
}

/*

         commc_base64_config_mime()
	       ---
	       returns MIME-compatible Base64 configuration
	       with line wrapping at 76 characters and CRLF
	       line endings for email and MIME content
	       encoding applications.

*/

commc_base64_config_t commc_base64_config_mime(void) {

    commc_base64_config_t config;

    config.use_padding   = 1;
    config.line_wrap     = 1;
    config.wrap_length   = 76;
    config.url_safe      = 0;
    config.strict_decode = 1;

    strcpy(config.line_ending, "\r\n");

    return config;
}

/*

         commc_base64_result_destroy()
	       ---
	       releases memory associated with Base64 result
	       structure including data buffer and metadata
	       to prevent memory leaks after decoding
	       operations and result processing.

*/

void commc_base64_result_destroy(commc_base64_result_t* result) {

    if (!result) {

        return;

    }

    if (result->data) {

        free(result->data);
        
    }

    free(result);
}

/* 
	==================================
           --- EOF ---
	==================================
*/