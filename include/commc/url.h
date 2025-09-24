/*

   ===================================
   C O M M C   -   U R L
   URL ENCODING AND DECODING
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_URL_H
#define COMMC_URL_H

#include "error.h"
#include <stdio.h>

/* 
	==================================
          --- TYPE DEFS ---
	==================================
*/

/*

	commc_url_components_t
	---
	structure containing the parsed components of
	a URL according to RFC 3986 specifications.
	provides easy access to scheme, authority,
	path, query, and fragment portions.

*/

typedef struct {

    char* scheme;       /* PROTOCOL (http, https, ftp, etc) */
    char* userinfo;     /* USERNAME[:PASSWORD] PORTION */
    char* host;         /* HOSTNAME OR IP ADDRESS */
    int   port;         /* PORT NUMBER (-1 IF NOT SPECIFIED) */
    char* path;         /* PATH COMPONENT */
    char* query;        /* QUERY STRING (AFTER ?) */
    char* fragment;     /* FRAGMENT (AFTER #) */

    char* full_url;     /* COMPLETE RECONSTRUCTED URL */
    int   is_relative;  /* FLAG FOR RELATIVE URL */

} commc_url_components_t;

/*

	commc_url_query_param_t
	---
	individual query string parameter with name
	and value components. supports empty values
	and handles URL decoding automatically for
	proper parameter extraction and manipulation.

*/

typedef struct {

    char* name;         /* PARAMETER NAME */
    char* value;        /* PARAMETER VALUE (NULL IF NONE) */

} commc_url_query_param_t;

/*

	commc_url_query_t
	---
	collection of query string parameters with
	dynamic array management. provides efficient
	access to URL query parameters with proper
	memory management and growth strategies.

*/

typedef struct {

    commc_url_query_param_t* params;    /* ARRAY OF PARAMETERS */
    int                      count;     /* NUMBER OF PARAMETERS */
    int                      capacity;  /* ALLOCATED CAPACITY */

} commc_url_query_t;

/*

	commc_url_encoding_mode_t
	---
	different URL encoding modes for various
	contexts and applications. controls which
	characters are encoded and how encoding
	is applied in different URL components.

*/

typedef enum {

    COMMC_URL_ENCODE_QUERY,      /* ENCODE FOR QUERY STRING PARAMETERS */
    COMMC_URL_ENCODE_PATH,       /* ENCODE FOR URL PATH COMPONENTS */
    COMMC_URL_ENCODE_FRAGMENT,   /* ENCODE FOR URL FRAGMENT PORTION */
    COMMC_URL_ENCODE_USERINFO,   /* ENCODE FOR USER INFO SECTION */
    COMMC_URL_ENCODE_FORM        /* ENCODE FOR FORM DATA (+ FOR SPACE) */

} commc_url_encoding_mode_t;

/* 
	==================================
         --- ENCODING API ---
	==================================
*/

/*

         commc_url_encode()
	       ---
	       encodes string for safe inclusion in URL
	       using percent encoding. handles special
	       characters according to RFC 3986 and
	       specified encoding mode requirements.

*/

char* commc_url_encode(const char* input, commc_url_encoding_mode_t mode);

/*

         commc_url_decode()
	       ---
	       decodes percent-encoded URL string back
	       to original form. handles all standard
	       escape sequences and validates proper
	       encoding format during decoding.

*/

char* commc_url_decode(const char* input);

/*

         commc_url_encode_component()
	       ---
	       encodes individual URL component with
	       component-specific rules and character
	       sets. provides fine-grained control over
	       encoding behavior for different URL parts.

*/

char* commc_url_encode_component(const char* input, commc_url_encoding_mode_t mode);

/*

         commc_url_decode_component()
	       ---
	       decodes individual URL component with
	       validation and error checking. returns
	       decoded string with proper null
	       termination and length validation.

*/

char* commc_url_decode_component(const char* input, int* decoded_length);

/* 
	==================================
         --- PARSING API ---
	==================================
*/

/*

         commc_url_parse()
	       ---
	       parses complete URL string into component
	       structure according to RFC 3986. handles
	       both absolute and relative URLs with
	       proper validation and normalization.

*/

commc_url_components_t* commc_url_parse(const char* url);

/*

         commc_url_components_destroy()
	       ---
	       releases all memory associated with URL
	       components structure including individual
	       component strings and container itself
	       to prevent memory leaks.

*/

void commc_url_components_destroy(commc_url_components_t* components);

/*

         commc_url_build()
	       ---
	       constructs complete URL string from
	       component structure with proper formatting
	       and validation. handles optional components
	       and generates RFC-compliant URL.

*/

char* commc_url_build(const commc_url_components_t* components);

/*

         commc_url_resolve()
	       ---
	       resolves relative URL against base URL
	       according to RFC 3986 resolution algorithm.
	       handles all relative reference types and
	       produces absolute URL result.

*/

char* commc_url_resolve(const char* base_url, const char* relative_url);

/*

         commc_url_normalize()
	       ---
	       normalizes URL by applying standard
	       transformations like case normalization,
	       path segment resolution, and default
	       port handling for consistent comparison.

*/

char* commc_url_normalize(const char* url);

/* 
	==================================
    --- QUERY STRING API ---
	==================================
*/

/*

         commc_url_query_parse()
	       ---
	       parses URL query string into parameter
	       structure with automatic URL decoding
	       and proper name-value pair separation
	       for easy parameter access.

*/

commc_url_query_t* commc_url_query_parse(const char* query_string);

/*

         commc_url_query_destroy()
	       ---
	       releases all memory associated with
	       query parameter structure including
	       parameter names, values, and container
	       to prevent memory leaks.

*/

void commc_url_query_destroy(commc_url_query_t* query);

/*

         commc_url_query_get()
	       ---
	       retrieves parameter value by name from
	       query structure. performs case-sensitive
	       lookup and returns first matching value
	       or NULL if parameter not found.

*/

const char* commc_url_query_get(const commc_url_query_t* query, const char* name);

/*

         commc_url_query_set()
	       ---
	       sets parameter value in query structure
	       with automatic memory management. updates
	       existing parameter or adds new one as
	       needed with proper string duplication.

*/

int commc_url_query_set(commc_url_query_t* query, const char* name, const char* value);

/*

         commc_url_query_remove()
	       ---
	       removes parameter from query structure
	       by name. handles memory cleanup and
	       array compaction to maintain structure
	       integrity after parameter removal.

*/

int commc_url_query_remove(commc_url_query_t* query, const char* name);

/*

         commc_url_query_build()
	       ---
	       constructs query string from parameter
	       structure with proper URL encoding and
	       formatting. generates standard query
	       string format for URL construction.

*/

char* commc_url_query_build(const commc_url_query_t* query);

/*

         commc_url_query_count()
	       ---
	       returns number of parameters in query
	       structure. useful for iteration and
	       capacity planning during query
	       manipulation operations.

*/

int commc_url_query_count(const commc_url_query_t* query);

/*

         commc_url_query_get_param()
	       ---
	       retrieves parameter structure by index
	       for iteration through all query parameters.
	       provides access to both name and value
	       components with bounds checking.

*/

const commc_url_query_param_t* commc_url_query_get_param(const commc_url_query_t* query, int index);

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_url_is_valid()
	       ---
	       validates URL format according to RFC 3986
	       without full parsing. performs syntax
	       checking and basic structure validation
	       for quick validity assessment.

*/

int commc_url_is_valid(const char* url);

/*

         commc_url_is_absolute()
	       ---
	       determines whether URL is absolute or
	       relative based on presence of scheme
	       component. useful for URL resolution
	       and validation logic.

*/

int commc_url_is_absolute(const char* url);

/*

         commc_url_get_scheme()
	       ---
	       extracts scheme component from URL without
	       full parsing. returns allocated string
	       containing protocol portion or NULL
	       if no scheme present.

*/

char* commc_url_get_scheme(const char* url);

/*

         commc_url_get_host()
	       ---
	       extracts host component from URL without
	       full parsing. handles IPv6 addresses,
	       domain names, and IPv4 addresses with
	       proper bracket handling.

*/

char* commc_url_get_host(const char* url);

/*

         commc_url_get_port()
	       ---
	       extracts port number from URL or returns
	       default port for known schemes. handles
	       explicit port specifications and standard
	       protocol default port assignment.

*/

int commc_url_get_port(const char* url);

/*

         commc_url_get_path()
	       ---
	       extracts path component from URL without
	       full parsing. returns allocated string
	       containing path portion with proper
	       handling of empty and root paths.

*/

char* commc_url_get_path(const char* url);

/*

         commc_url_equals()
	       ---
	       compares two URLs for equality after
	       normalization. handles minor formatting
	       differences and provides semantic
	       comparison rather than string matching.

*/

int commc_url_equals(const char* url1, const char* url2);

/*

         commc_url_join()
	       ---
	       joins base URL with path component using
	       proper path resolution rules. handles
	       trailing slashes and path normalization
	       for consistent URL construction.

*/

char* commc_url_join(const char* base, const char* path);

/* 
	==================================
           --- EOF ---
	==================================
*/

#endif /* COMMC_URL_H */