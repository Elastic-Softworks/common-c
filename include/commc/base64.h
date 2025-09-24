/*

   ===================================
   C O M M C   -   B A S E 6 4
   BASE64 ENCODING AND DECODING
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_BASE64_H
#define COMMC_BASE64_H

#include "error.h"
#include <stdio.h>

/* 
	==================================
          --- TYPE DEFS ---
	==================================
*/

/*

	commc_base64_config_t
	---
	configuration structure for Base64 encoding behavior.
	allows customization of padding, character set, and
	line wrapping for different Base64 variants and
	compatibility requirements.

*/

typedef struct {

    int  use_padding;       /* INCLUDE PADDING CHARACTERS (=) */
    int  line_wrap;         /* WRAP LINES AT SPECIFIED LENGTH */
    int  wrap_length;       /* CHARACTERS PER LINE (0 = NO WRAP) */
    char line_ending[4];    /* LINE ENDING SEQUENCE (\n, \r\n, etc) */
    
    int  url_safe;          /* USE URL-SAFE ALPHABET (- and _) */
    int  strict_decode;     /* REJECT INVALID CHARACTERS */

} commc_base64_config_t;

/*

	commc_base64_encoder_t
	---
	streaming Base64 encoder context that maintains
	state for incremental encoding of large binary
	data streams with proper buffering and output
	management for efficient processing.

*/

typedef struct {

    commc_base64_config_t  config;     /* ENCODING CONFIGURATION */
    
    unsigned char          buffer[3];  /* INPUT BYTE TRIPLET BUFFER */
    int                    buf_pos;    /* CURRENT BUFFER POSITION */
    
    char*                  output;     /* OUTPUT STRING BUFFER */
    int                    out_size;   /* OUTPUT BUFFER SIZE */
    int                    out_pos;    /* CURRENT OUTPUT POSITION */
    
    int                    line_pos;   /* CURRENT LINE POSITION */
    int                    total_in;   /* TOTAL INPUT BYTES */
    int                    total_out;  /* TOTAL OUTPUT CHARACTERS */
    
    commc_error_context_t  error;      /* ERROR CONTEXT INFORMATION */

} commc_base64_encoder_t;

/*

	commc_base64_decoder_t
	---
	streaming Base64 decoder context for incremental
	decoding with validation and error handling.
	maintains proper state for character validation,
	padding detection, and binary reconstruction.

*/

typedef struct {

    commc_base64_config_t  config;     /* DECODING CONFIGURATION */
    
    unsigned char          buffer[4];  /* INPUT CHARACTER QUARTET BUFFER */
    int                    buf_pos;    /* CURRENT BUFFER POSITION */
    
    unsigned char*         output;     /* OUTPUT BINARY BUFFER */
    int                    out_size;   /* OUTPUT BUFFER SIZE */
    int                    out_pos;    /* CURRENT OUTPUT POSITION */
    
    int                    padding_count; /* NUMBER OF PADDING CHARS SEEN */
    int                    finished;      /* DECODING COMPLETE FLAG */
    int                    total_in;      /* TOTAL INPUT CHARACTERS */
    int                    total_out;     /* TOTAL OUTPUT BYTES */
    
    commc_error_context_t  error;      /* ERROR CONTEXT INFORMATION */

} commc_base64_decoder_t;

/*

	commc_base64_result_t
	---
	result structure for Base64 encoding/decoding
	operations containing output data, size
	information, and status indicators for
	proper result handling and validation.

*/

typedef struct {

    unsigned char* data;        /* OUTPUT DATA BUFFER */
    int            size;        /* SIZE OF OUTPUT DATA */
    int            capacity;    /* ALLOCATED BUFFER CAPACITY */
    
    int            input_consumed;  /* BYTES/CHARS PROCESSED */
    int            is_complete;     /* OPERATION COMPLETE FLAG */

} commc_base64_result_t;

/* 
	==================================
         --- ENCODER API ---
	==================================
*/

/*

         commc_base64_encoder_create()
	       ---
	       creates and initializes Base64 encoder with
	       specified configuration and output buffer
	       capacity. sets up streaming context for
	       incremental binary data encoding.

*/

commc_base64_encoder_t* commc_base64_encoder_create(const commc_base64_config_t* config, int output_capacity);

/*

         commc_base64_encoder_destroy()
	       ---
	       releases all resources associated with Base64
	       encoder including output buffers and state
	       information. prevents memory leaks during
	       encoding operations and cleanup.

*/

void commc_base64_encoder_destroy(commc_base64_encoder_t* encoder);

/*

         commc_base64_encoder_feed()
	       ---
	       feeds binary data to encoder for processing.
	       handles byte triplet buffering and generates
	       Base64 output characters as complete groups
	       become available during streaming.

*/

int commc_base64_encoder_feed(commc_base64_encoder_t* encoder, const unsigned char* data, int length);

/*

         commc_base64_encoder_finish()
	       ---
	       completes encoding process by handling final
	       partial byte groups and adding appropriate
	       padding. generates remaining output and
	       finalizes encoded string construction.

*/

int commc_base64_encoder_finish(commc_base64_encoder_t* encoder);

/*

         commc_base64_encoder_get_output()
	       ---
	       retrieves current encoded output from encoder
	       buffer. provides access to accumulated Base64
	       string with proper null termination for
	       string handling and transmission.

*/

const char* commc_base64_encoder_get_output(const commc_base64_encoder_t* encoder);

/*

         commc_base64_encoder_reset()
	       ---
	       resets encoder to initial state for reuse
	       with new data. clears buffers and counters
	       while preserving configuration settings
	       for efficient encoder recycling.

*/

void commc_base64_encoder_reset(commc_base64_encoder_t* encoder);

/* 
	==================================
         --- DECODER API ---
	==================================
*/

/*

         commc_base64_decoder_create()
	       ---
	       creates and initializes Base64 decoder with
	       specified configuration and output buffer
	       capacity. sets up streaming context for
	       incremental Base64 string decoding.

*/

commc_base64_decoder_t* commc_base64_decoder_create(const commc_base64_config_t* config, int output_capacity);

/*

         commc_base64_decoder_destroy()
	       ---
	       releases all resources associated with Base64
	       decoder including output buffers and state
	       information. prevents memory leaks during
	       decoding operations and cleanup.

*/

void commc_base64_decoder_destroy(commc_base64_decoder_t* decoder);

/*

         commc_base64_decoder_feed()
	       ---
	       feeds Base64 characters to decoder for
	       processing. handles character validation,
	       quartet buffering, and generates binary
	       output as complete groups are decoded.

*/

int commc_base64_decoder_feed(commc_base64_decoder_t* decoder, const char* data, int length);

/*

         commc_base64_decoder_finish()
	       ---
	       completes decoding process by handling final
	       character group and validating padding.
	       generates remaining binary output and
	       performs final validation checks.

*/

int commc_base64_decoder_finish(commc_base64_decoder_t* decoder);

/*

         commc_base64_decoder_get_output()
	       ---
	       retrieves current decoded binary output from
	       decoder buffer. provides access to accumulated
	       byte data with proper size information for
	       binary data handling and processing.

*/

const unsigned char* commc_base64_decoder_get_output(const commc_base64_decoder_t* decoder, int* size);

/*

         commc_base64_decoder_reset()
	       ---
	       resets decoder to initial state for reuse
	       with new data. clears buffers and counters
	       while preserving configuration settings
	       for efficient decoder recycling.

*/

void commc_base64_decoder_reset(commc_base64_decoder_t* decoder);

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_base64_encode()
	       ---
	       encodes binary data to Base64 string in a
	       single operation. convenient wrapper for
	       simple encoding tasks without streaming
	       requirements or complex configuration.

*/

char* commc_base64_encode(const unsigned char* data, int length, const commc_base64_config_t* config);

/*

         commc_base64_decode()
	       ---
	       decodes Base64 string to binary data in a
	       single operation. convenient wrapper for
	       simple decoding tasks without streaming
	       requirements or incremental processing.

*/

commc_base64_result_t* commc_base64_decode(const char* data, int length, const commc_base64_config_t* config);

/*

         commc_base64_get_encoded_size()
	       ---
	       calculates the encoded size for given input
	       length considering padding and line wrapping
	       requirements. useful for buffer allocation
	       and capacity planning in encoding operations.

*/

int commc_base64_get_encoded_size(int input_length, const commc_base64_config_t* config);

/*

         commc_base64_get_decoded_size()
	       ---
	       estimates decoded size for Base64 input
	       considering padding and format validation.
	       provides upper bound for buffer allocation
	       during decoding operations.

*/

int commc_base64_get_decoded_size(int input_length, const commc_base64_config_t* config);

/*

         commc_base64_validate()
	       ---
	       validates Base64 string format according to
	       configuration requirements. checks character
	       validity, padding correctness, and overall
	       format compliance before decoding attempts.

*/

int commc_base64_validate(const char* data, int length, const commc_base64_config_t* config);

/*

         commc_base64_config_default()
	       ---
	       returns standard Base64 configuration with
	       padding enabled, standard alphabet, and no
	       line wrapping. suitable for most general
	       Base64 encoding and decoding operations.

*/

commc_base64_config_t commc_base64_config_default(void);

/*

         commc_base64_config_url_safe()
	       ---
	       returns URL-safe Base64 configuration using
	       modified alphabet with - and _ characters
	       instead of + and /. typically used without
	       padding for URL and filename compatibility.

*/

commc_base64_config_t commc_base64_config_url_safe(void);

/*

         commc_base64_config_mime()
	       ---
	       returns MIME-compatible Base64 configuration
	       with line wrapping at 76 characters and CRLF
	       line endings. suitable for email and MIME
	       content encoding applications.

*/

commc_base64_config_t commc_base64_config_mime(void);

/*

         commc_base64_result_destroy()
	       ---
	       releases memory associated with Base64 result
	       structure including data buffer and metadata.
	       prevents memory leaks after decoding
	       operations and result processing.

*/

void commc_base64_result_destroy(commc_base64_result_t* result);

/* 
	==================================
           --- EOF ---
	==================================
*/

#endif /* COMMC_BASE64_H */