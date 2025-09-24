/* 	
   ===================================
   E M A I L . H
   BASIC EMAIL PARSING HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    basic email message parsing and processing
	    capabilities for RFC 5322 compliant email
	    messages. provides header extraction, body
	    parsing, and MIME content handling.
	       
	    designed for educational clarity with
	    production-ready error handling and
	    C89 compliance throughout.

*/

#ifndef COMMC_EMAIL_H
#define COMMC_EMAIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include <stddef.h>
#include "error.h"

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#define COMMC_EMAIL_MAX_HEADER_NAME_LENGTH   64
#define COMMC_EMAIL_MAX_HEADER_VALUE_LENGTH  512
#define COMMC_EMAIL_MAX_ADDRESS_LENGTH       256
#define COMMC_EMAIL_MAX_SUBJECT_LENGTH       512
#define COMMC_EMAIL_MAX_HEADERS              50
#define COMMC_EMAIL_MAX_RECIPIENTS           50
#define COMMC_EMAIL_DEFAULT_BUFFER_SIZE      4096

/* 
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_email_header_t
	       ---
	       represents a single email header
	       field with name and value.

*/

typedef struct {

    char name[COMMC_EMAIL_MAX_HEADER_NAME_LENGTH];
    char value[COMMC_EMAIL_MAX_HEADER_VALUE_LENGTH];

} commc_email_header_t;

/*

         commc_email_address_t  
	       ---
	       represents an email address with
	       optional display name.

*/

typedef struct {

    char email[COMMC_EMAIL_MAX_ADDRESS_LENGTH];
    char name[COMMC_EMAIL_MAX_ADDRESS_LENGTH];

} commc_email_address_t;

/*

         commc_email_message_t
	       ---
	       comprehensive email message structure
	       containing headers, addresses, and body.

*/

typedef struct {

    /* Header collection */
    
    commc_email_header_t headers[COMMC_EMAIL_MAX_HEADERS];
    int                  header_count;
    
    /* Primary addresses */
    
    commc_email_address_t from;
    commc_email_address_t reply_to;
    commc_email_address_t return_path;
    
    /* Recipients */
    
    commc_email_address_t to[COMMC_EMAIL_MAX_RECIPIENTS];
    int                   to_count;
    
    commc_email_address_t cc[COMMC_EMAIL_MAX_RECIPIENTS];  
    int                   cc_count;
    
    commc_email_address_t bcc[COMMC_EMAIL_MAX_RECIPIENTS];
    int                   bcc_count;
    
    /* Message content */
    
    char  subject[COMMC_EMAIL_MAX_SUBJECT_LENGTH];
    char* body;
    size_t body_length;
    
    /* Content metadata */
    
    char  content_type[64];
    char  content_encoding[32];
    char  message_id[128];
    char  date[64];
    
    /* Parsing state */
    
    int   is_multipart;
    char  boundary[128];

} commc_email_message_t;

/*

         commc_email_parser_t
	       ---
	       email parser context for processing
	       raw email data incrementally.

*/

typedef struct {

    char*  buffer;
    size_t buffer_size;
    size_t buffer_pos;
    
    /* Parser state */
    
    int    parsing_headers;
    int    headers_complete;
    size_t header_start;
    size_t body_start;
    
    /* Current parsing context */
    
    int    current_header_index;
    char   current_header_name[COMMC_EMAIL_MAX_HEADER_NAME_LENGTH];
    
    /* Error tracking */
    
    int    has_error;
    char   error_message[256];

} commc_email_parser_t;

/* 
	==================================
        --- MESSAGE MANAGEMENT ---
	==================================
*/

/*

         commc_email_message_create()
	       ---
	       creates and initializes a new email
	       message structure.

*/

commc_error_t commc_email_message_create(commc_email_message_t** message);

/*

         commc_email_message_destroy()
	       ---
	       destroys email message and frees
	       all associated memory.

*/

void commc_email_message_destroy(commc_email_message_t* message);

/*

         commc_email_message_clear()
	       ---
	       clears email message content while
	       preserving structure allocation.

*/

commc_error_t commc_email_message_clear(commc_email_message_t* message);

/* 
	==================================
        --- PARSER MANAGEMENT ---
	==================================
*/

/*

         commc_email_parser_create()
	       ---
	       creates email parser with specified
	       buffer size for incremental parsing.

*/

commc_error_t commc_email_parser_create(commc_email_parser_t** parser,
                                        size_t                buffer_size);

/*

         commc_email_parser_destroy()
	       ---
	       destroys email parser and cleans
	       up all resources.

*/

void commc_email_parser_destroy(commc_email_parser_t* parser);

/*

         commc_email_parser_reset()
	       ---
	       resets parser state for parsing
	       a new email message.

*/

commc_error_t commc_email_parser_reset(commc_email_parser_t* parser);

/* 
	==================================
           --- PARSING FUNCTIONS ---
	==================================
*/

/*

         commc_email_parse_message()
	       ---
	       parses complete email message from
	       string buffer into message structure.

*/

commc_error_t commc_email_parse_message(const char*            email_data,
                                        size_t                 data_length,
                                        commc_email_message_t* message);

/*

         commc_email_parse_headers()
	       ---
	       parses only email headers from
	       message data.

*/

commc_error_t commc_email_parse_headers(const char*            header_data,
                                        size_t                 data_length,
                                        commc_email_message_t* message);

/*

         commc_email_parse_address()
	       ---
	       parses email address from string
	       format "Display Name <email@domain.com>".

*/

commc_error_t commc_email_parse_address(const char*            address_string,
                                        commc_email_address_t* address);

/*

         commc_email_parse_address_list()
	       ---
	       parses comma-separated list of
	       email addresses.

*/

commc_error_t commc_email_parse_address_list(const char*             address_list,
                                             commc_email_address_t*  addresses,
                                             int                     max_addresses,
                                             int*                    address_count);

/* 
	==================================
        --- HEADER FUNCTIONS ---
	==================================
*/

/*

         commc_email_get_header()
	       ---
	       retrieves header value by name
	       from email message.

*/

commc_error_t commc_email_get_header(const commc_email_message_t* message,
                                     const char*                  header_name,
                                     char*                        header_value,
                                     size_t                       value_buffer_size);

/*

         commc_email_set_header()
	       ---
	       sets or updates header value
	       in email message.

*/

commc_error_t commc_email_set_header(commc_email_message_t* message,
                                     const char*            header_name,
                                     const char*            header_value);

/*

         commc_email_add_header()
	       ---
	       adds new header to email message
	       without replacing existing.

*/

commc_error_t commc_email_add_header(commc_email_message_t* message,
                                     const char*            header_name,
                                     const char*            header_value);

/*

         commc_email_remove_header()
	       ---
	       removes header from email message
	       by name.

*/

commc_error_t commc_email_remove_header(commc_email_message_t* message,
                                        const char*            header_name);

/* 
	==================================
        --- ADDRESS FUNCTIONS ---
	==================================
*/

/*

         commc_email_add_recipient()
	       ---
	       adds recipient to TO field
	       of email message.

*/

commc_error_t commc_email_add_recipient(commc_email_message_t*       message,
                                        const commc_email_address_t* recipient);

/*

         commc_email_add_cc_recipient()
	       ---
	       adds recipient to CC field
	       of email message.

*/

commc_error_t commc_email_add_cc_recipient(commc_email_message_t*       message,
                                           const commc_email_address_t* recipient);

/*

         commc_email_add_bcc_recipient()
	       ---
	       adds recipient to BCC field
	       of email message.

*/

commc_error_t commc_email_add_bcc_recipient(commc_email_message_t*       message,
                                            const commc_email_address_t* recipient);

/*

         commc_email_set_from()
	       ---
	       sets FROM address of email message.

*/

commc_error_t commc_email_set_from(commc_email_message_t*       message,
                                   const commc_email_address_t* from_address);

/*

         commc_email_set_reply_to()
	       ---
	       sets REPLY-TO address of email message.

*/

commc_error_t commc_email_set_reply_to(commc_email_message_t*       message,
                                       const commc_email_address_t* reply_address);

/* 
	==================================
        --- CONTENT FUNCTIONS ---
	==================================
*/

/*

         commc_email_set_subject()
	       ---
	       sets subject line of email message.

*/

commc_error_t commc_email_set_subject(commc_email_message_t* message,
                                      const char*            subject);

/*

         commc_email_set_body()
	       ---
	       sets body content of email message
	       with specified content type.

*/

commc_error_t commc_email_set_body(commc_email_message_t* message,
                                   const char*            body_text,
                                   const char*            content_type);

/*

         commc_email_get_body_text()
	       ---
	       retrieves plain text body content
	       from email message.

*/

commc_error_t commc_email_get_body_text(const commc_email_message_t* message,
                                        char*                        text_buffer,
                                        size_t                       buffer_size);

/* 
	==================================
        --- VALIDATION FUNCTIONS ---
	==================================
*/

/*

         commc_email_validate_address()
	       ---
	       validates email address format
	       according to basic RFC rules.

*/

commc_error_t commc_email_validate_address(const char* email_address);

/*

         commc_email_validate_message()
	       ---
	       validates email message structure
	       and required headers.

*/

commc_error_t commc_email_validate_message(const commc_email_message_t* message);

/* 
	==================================
        --- UTILITY FUNCTIONS ---
	==================================
*/

/*

         commc_email_format_address()
	       ---
	       formats email address into standard
	       "Display Name <email>" format.

*/

commc_error_t commc_email_format_address(const commc_email_address_t* address,
                                         char*                        formatted_address,
                                         size_t                       buffer_size);

/*

         commc_email_decode_header()
	       ---
	       decodes RFC 2047 encoded headers
	       (=?charset?encoding?encoded-text?=).

*/

commc_error_t commc_email_decode_header(const char* encoded_header,
                                        char*       decoded_header,
                                        size_t      buffer_size);

/*

         commc_email_encode_header()
	       ---
	       encodes header value using RFC 2047
	       encoding when necessary.

*/

commc_error_t commc_email_encode_header(const char* header_value,
                                        const char* charset,
                                        char*       encoded_header,
                                        size_t      buffer_size);

/*

         commc_email_get_content_type()
	       ---
	       extracts main content type from
	       Content-Type header value.

*/

commc_error_t commc_email_get_content_type(const char* content_type_header,
                                           char*       content_type,
                                           size_t      type_buffer_size);

/*

         commc_email_extract_boundary()
	       ---
	       extracts boundary parameter from
	       multipart Content-Type header.

*/

commc_error_t commc_email_extract_boundary(const char* content_type_header,
                                           char*       boundary,
                                           size_t      boundary_buffer_size);

/* 
	==================================
        --- CONVENIENCE FUNCTIONS ---
	==================================
*/

/*

         commc_email_create_simple()
	       ---
	       creates simple text email message
	       with basic headers and body.

*/

commc_error_t commc_email_create_simple(commc_email_message_t** message,
                                        const char*             from_email,
                                        const char*             from_name,
                                        const char*             to_email,
                                        const char*             to_name,
                                        const char*             subject,
                                        const char*             body_text);

/*

         commc_email_parse_from_file()
	       ---
	       parses email message from file
	       on disk.

*/

commc_error_t commc_email_parse_from_file(const char*            filename,
                                          commc_email_message_t* message);

/*

         commc_email_save_to_file()
	       ---
	       saves email message to file
	       in RFC 5322 format.

*/

commc_error_t commc_email_save_to_file(const commc_email_message_t* message,
                                       const char*                  filename);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_EMAIL_H */

/* 
	==================================
             --- EOF ---
	==================================
*/