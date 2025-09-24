/* 	
   ===================================
   H T T P . H 
   HTTP/1.1 CLIENT IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module provides a comprehensive HTTP/1.1 client
	    implementation with support for GET, POST, PUT, DELETE
	    and other HTTP methods. handles headers, cookies,
	    redirects, and various response formats.
	       
	    designed to simplify web API integration while
	    maintaining C89 compliance and cross-platform
	    socket compatibility.

*/

#ifndef COMMC_HTTP_H
#define COMMC_HTTP_H

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

#define COMMC_HTTP_MAX_URL_LENGTH      2048   /* MAXIMUM URL LENGTH */
#define COMMC_HTTP_MAX_HEADER_LENGTH   4096   /* MAXIMUM HEADER LENGTH */
#define COMMC_HTTP_MAX_BODY_LENGTH     65536  /* MAXIMUM BODY LENGTH */
#define COMMC_HTTP_MAX_HEADERS         64     /* MAXIMUM HEADER COUNT */
#define COMMC_HTTP_DEFAULT_TIMEOUT     30     /* DEFAULT TIMEOUT SECONDS */
#define COMMC_HTTP_DEFAULT_PORT        80     /* DEFAULT HTTP PORT */
#define COMMC_HTTP_DEFAULT_HTTPS_PORT  443    /* DEFAULT HTTPS PORT */

/* 
	==================================
             --- ENUMS ---
	==================================
*/

/*

         commc_http_method_t
	       ---
	       enumeration defining supported HTTP request
	       methods for client operations.

*/

typedef enum {

    COMMC_HTTP_GET,        /* GET METHOD */
    COMMC_HTTP_POST,       /* POST METHOD */
    COMMC_HTTP_PUT,        /* PUT METHOD */
    COMMC_HTTP_DELETE,     /* DELETE METHOD */
    COMMC_HTTP_HEAD,       /* HEAD METHOD */
    COMMC_HTTP_OPTIONS,    /* OPTIONS METHOD */
    COMMC_HTTP_PATCH       /* PATCH METHOD */
    
} commc_http_method_t;

/*

         commc_http_version_t
	       ---
	       enumeration specifying HTTP protocol versions
	       supported by the client implementation.

*/

typedef enum {

    COMMC_HTTP_VERSION_1_0,    /* HTTP/1.0 */
    COMMC_HTTP_VERSION_1_1     /* HTTP/1.1 (DEFAULT) */
    
} commc_http_version_t;

/*

         commc_http_status_class_t
	       ---
	       enumeration categorizing HTTP response status
	       codes into standard classes for easy handling.

*/

typedef enum {

    COMMC_HTTP_STATUS_INFORMATIONAL,  /* 1XX CODES */
    COMMC_HTTP_STATUS_SUCCESS,        /* 2XX CODES */
    COMMC_HTTP_STATUS_REDIRECTION,    /* 3XX CODES */
    COMMC_HTTP_STATUS_CLIENT_ERROR,   /* 4XX CODES */
    COMMC_HTTP_STATUS_SERVER_ERROR    /* 5XX CODES */
    
} commc_http_status_class_t;

/* 
	==================================
           --- STRUCTURES ---
	==================================
*/

/*

         commc_http_header_t
	       ---
	       structure representing a single HTTP header
	       with name and value fields.

*/

typedef struct {

    char name[256];     /* HEADER NAME */
    char value[1024];   /* HEADER VALUE */

} commc_http_header_t;

/*

         commc_http_url_t
	       ---
	       structure containing parsed URL components
	       for HTTP request construction.

*/

typedef struct {

    char protocol[16];   /* HTTP OR HTTPS */
    char hostname[256];  /* SERVER HOSTNAME */
    int  port;          /* SERVER PORT */
    char path[1024];    /* REQUEST PATH */
    char query[1024];   /* QUERY STRING */
    char fragment[256]; /* URL FRAGMENT */

} commc_http_url_t;

/*

         commc_http_request_t
	       ---
	       structure representing a complete HTTP request
	       with method, headers, and body content.

*/

typedef struct {

    commc_http_method_t  method;                         /* REQUEST METHOD */
    commc_http_version_t version;                        /* HTTP VERSION */
    
    commc_http_url_t     url;                            /* TARGET URL */
    
    commc_http_header_t  headers[COMMC_HTTP_MAX_HEADERS]; /* REQUEST HEADERS */
    int                  header_count;                    /* HEADER COUNT */
    
    char*                body;                            /* REQUEST BODY */
    size_t               body_length;                     /* BODY LENGTH */
    
    int                  timeout_seconds;                 /* REQUEST TIMEOUT */
    int                  follow_redirects;                /* REDIRECT FLAG */
    int                  max_redirects;                   /* MAX REDIRECTS */

} commc_http_request_t;

/*

         commc_http_response_t
	       ---
	       structure containing HTTP response data
	       including status, headers, and body content.

*/

typedef struct {

    commc_http_version_t version;                         /* HTTP VERSION */
    
    int                  status_code;                     /* STATUS CODE */
    char                 status_message[128];             /* STATUS MESSAGE */
    commc_http_status_class_t status_class;               /* STATUS CLASS */
    
    commc_http_header_t  headers[COMMC_HTTP_MAX_HEADERS]; /* RESPONSE HEADERS */
    int                  header_count;                    /* HEADER COUNT */
    
    char*                body;                            /* RESPONSE BODY */
    size_t               body_length;                     /* BODY LENGTH */
    size_t               body_capacity;                   /* ALLOCATED SIZE */
    
    double               response_time;                   /* RESPONSE TIME MS */
    int                  redirected;                      /* REDIRECT FLAG */
    char                 final_url[COMMC_HTTP_MAX_URL_LENGTH]; /* FINAL URL */

} commc_http_response_t;

/*

         commc_http_client_t
	       ---
	       main HTTP client structure containing connection
	       state and configuration options.

*/

typedef struct {

    int socket_fd;                                        /* SOCKET DESCRIPTOR */
    
    char user_agent[256];                                 /* USER AGENT STRING */
    int  connection_timeout;                              /* CONNECTION TIMEOUT */
    int  request_timeout;                                 /* REQUEST TIMEOUT */
    
    int  keep_alive;                                      /* KEEP-ALIVE FLAG */
    int  follow_redirects;                                /* REDIRECT HANDLING */
    int  max_redirects;                                   /* MAX REDIRECTS */
    
    commc_http_header_t default_headers[COMMC_HTTP_MAX_HEADERS]; /* DEFAULT HEADERS */
    int                 default_header_count;             /* DEFAULT HEADER COUNT */
    
    char last_error[512];                                 /* LAST ERROR MESSAGE */

} commc_http_client_t;

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_http_client_create()
	       ---
	       creates and initializes a new HTTP client
	       with default configuration settings.

*/

commc_error_t commc_http_client_create(commc_http_client_t** client);

/*

         commc_http_client_destroy()
	       ---
	       destroys an HTTP client and frees all
	       associated resources including active connections.

*/

void commc_http_client_destroy(commc_http_client_t* client);

/*

         commc_http_client_set_user_agent()
	       ---
	       sets the User-Agent header value for all
	       requests made by the client.

*/

commc_error_t commc_http_client_set_user_agent(commc_http_client_t* client,
                                                const char*          user_agent);

/*

         commc_http_client_set_timeout()
	       ---
	       configures connection and request timeout
	       values for the HTTP client.

*/

commc_error_t commc_http_client_set_timeout(commc_http_client_t* client,
                                             int                  connection_timeout,
                                             int                  request_timeout);

/*

         commc_http_client_add_default_header()
	       ---
	       adds a default header that will be included
	       in all requests made by the client.

*/

commc_error_t commc_http_client_add_default_header(commc_http_client_t* client,
                                                    const char*          name,
                                                    const char*          value);

/* 
	==================================
           --- REQUESTS ---
	==================================
*/

/*

         commc_http_request_create()
	       ---
	       creates and initializes a new HTTP request
	       structure with default values.

*/

commc_error_t commc_http_request_create(commc_http_request_t** request);

/*

         commc_http_request_destroy()
	       ---
	       destroys an HTTP request and frees all
	       associated memory including body content.

*/

void commc_http_request_destroy(commc_http_request_t* request);

/*

         commc_http_request_set_method()
	       ---
	       sets the HTTP method for the request
	       (GET, POST, PUT, DELETE, etc.).

*/

commc_error_t commc_http_request_set_method(commc_http_request_t* request,
                                             commc_http_method_t   method);

/*

         commc_http_request_set_url()
	       ---
	       sets the target URL for the HTTP request,
	       parsing it into component parts.

*/

commc_error_t commc_http_request_set_url(commc_http_request_t* request,
                                          const char*           url);

/*

         commc_http_request_add_header()
	       ---
	       adds a header to the HTTP request with
	       automatic duplicate handling.

*/

commc_error_t commc_http_request_add_header(commc_http_request_t* request,
                                             const char*           name,
                                             const char*           value);

/*

         commc_http_request_set_body()
	       ---
	       sets the request body content with automatic
	       Content-Length header calculation.

*/

commc_error_t commc_http_request_set_body(commc_http_request_t* request,
                                           const char*           body,
                                           size_t                body_length);

/*

         commc_http_request_set_form_data()
	       ---
	       sets the request body as URL-encoded form data,
	       automatically setting the appropriate content type.

*/

commc_error_t commc_http_request_set_form_data(commc_http_request_t* request,
                                                const char*           form_data);

/*

         commc_http_request_set_json()
	       ---
	       sets the request body as JSON content with
	       automatic Content-Type header setting.

*/

commc_error_t commc_http_request_set_json(commc_http_request_t* request,
                                           const char*           json_data);

/* 
	==================================
           --- RESPONSES ---
	==================================
*/

/*

         commc_http_response_create()
	       ---
	       creates and initializes a new HTTP response
	       structure for storing server responses.

*/

commc_error_t commc_http_response_create(commc_http_response_t** response);

/*

         commc_http_response_destroy()
	       ---
	       destroys an HTTP response and frees all
	       associated memory including body content.

*/

void commc_http_response_destroy(commc_http_response_t* response);

/*

         commc_http_response_get_header()
	       ---
	       retrieves a specific header value from the
	       HTTP response by header name.

*/

const char* commc_http_response_get_header(commc_http_response_t* response,
                                           const char*            header_name);

/*

         commc_http_response_get_status_class()
	       ---
	       returns the status class (success, error, etc.)
	       for the HTTP response status code.

*/

commc_http_status_class_t commc_http_response_get_status_class(commc_http_response_t* response);

/* 
	==================================
           --- EXECUTION ---
	==================================
*/

/*

         commc_http_client_execute()
	       ---
	       executes an HTTP request using the client,
	       handling connection, transmission, and response
	       processing with timeout and redirect support.

*/

commc_error_t commc_http_client_execute(commc_http_client_t*  client,
                                         commc_http_request_t* request,
                                         commc_http_response_t* response);

/*

         commc_http_get()
	       ---
	       convenience function for executing a simple
	       HTTP GET request with minimal setup.

*/

commc_error_t commc_http_get(const char*            url,
                              commc_http_response_t* response);

/*

         commc_http_post()
	       ---
	       convenience function for executing an HTTP POST
	       request with body content.

*/

commc_error_t commc_http_post(const char*            url,
                               const char*            body,
                               size_t                 body_length,
                               commc_http_response_t* response);

/*

         commc_http_post_json()
	       ---
	       convenience function for posting JSON data
	       with automatic content-type handling.

*/

commc_error_t commc_http_post_json(const char*            url,
                                    const char*            json_data,
                                    commc_http_response_t* response);

/*

         commc_http_post_form()
	       ---
	       convenience function for posting form data
	       with URL encoding and content-type setting.

*/

commc_error_t commc_http_post_form(const char*            url,
                                    const char*            form_data,
                                    commc_http_response_t* response);

/* 
	==================================
           --- UTILITIES ---
	==================================
*/

/*

         commc_http_url_parse()
	       ---
	       parses a URL string into component parts
	       including protocol, host, port, and path.

*/

commc_error_t commc_http_url_parse(const char*       url_string,
                                   commc_http_url_t* parsed_url);

/*

         commc_http_url_encode()
	       ---
	       URL-encodes a string for safe transmission
	       in HTTP requests and query parameters.

*/

commc_error_t commc_http_url_encode(const char* input,
                                    char*       output,
                                    size_t      output_size);

/*

         commc_http_url_decode()
	       ---
	       URL-decodes a string, converting percent-encoded
	       characters back to their original form.

*/

commc_error_t commc_http_url_decode(const char* input,
                                    char*       output,
                                    size_t      output_size);

/*

         commc_http_method_to_string()
	       ---
	       converts an HTTP method enumeration value
	       to its string representation.

*/

const char* commc_http_method_to_string(commc_http_method_t method);

/*

         commc_http_status_to_string()
	       ---
	       converts an HTTP status code to its
	       standard reason phrase string.

*/

const char* commc_http_status_to_string(int status_code);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_HTTP_H */

/* 
	==================================
             --- EOF ---
	==================================
*/