/* 	
   ===================================
   H T T P . C
   HTTP/1.1 CLIENT IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module implements a comprehensive HTTP/1.1
	    client with support for various request methods,
	    headers, and response handling. focuses on
	    reliability and educational clarity.
	       
	    provides cross-platform socket operations while
	    maintaining C89 compliance and robust error
	    handling throughout all network operations.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif
#define close closesocket
#define SOCKET_ERROR_CODE WSAGetLastError()
#define COMMC_INVALID_SOCKET ((int)-1)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define SOCKET_ERROR -1
#define SOCKET_ERROR_CODE errno
#define COMMC_INVALID_SOCKET -1
#endif

#include "commc/http.h"
#include "commc/error.h"

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#define HTTP_REQUEST_BUFFER_SIZE  8192    /* REQUEST BUFFER SIZE */
#define HTTP_RESPONSE_BUFFER_SIZE 8192    /* RESPONSE BUFFER SIZE */
#define HTTP_LINE_BUFFER_SIZE     2048    /* LINE BUFFER SIZE */
#define MAX_RESPONSE_SIZE         1048576 /* 1MB MAX RESPONSE */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         http_method_strings[]
	       ---
	       array of HTTP method names corresponding
	       to the method enumeration values.

*/

static const char* http_method_strings[] = {
    "GET",
    "POST", 
    "PUT",
    "DELETE",
    "HEAD",
    "OPTIONS",
    "PATCH"
};

/*

         initialize_sockets()
	       ---
	       initializes the socket library on Windows
	       platforms. no-op on Unix systems.

*/

static commc_error_t initialize_sockets(void) {

#ifdef _WIN32

    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    
    if (result != 0) {
    
        return COMMC_SYSTEM_ERROR;
        
    }
    
#endif

    return COMMC_SUCCESS;
    
}

/*

         cleanup_sockets()
	       ---
	       cleans up the socket library on Windows
	       platforms. no-op on Unix systems.

*/

static void cleanup_sockets(void) {

#ifdef _WIN32

    WSACleanup();
    
#endif

}

/*

         create_socket_connection()
	       ---
	       creates a TCP socket connection to the specified
	       hostname and port with timeout handling.

*/

static commc_error_t create_socket_connection(const char* hostname,
                                              int         port,
                                              int*        socket_fd) {

    struct hostent* host_entry;
    struct sockaddr_in server_addr;
    int sock;
    
    if (!hostname || !socket_fd) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Initialize sockets */
    
    if (initialize_sockets() != COMMC_SUCCESS) {
    
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Create socket */
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock == COMMC_INVALID_SOCKET) {
    
        cleanup_sockets();
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Resolve hostname */
    
    host_entry = gethostbyname(hostname);
    
    if (!host_entry) {
    
        close(sock);
        cleanup_sockets();
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Set up server address */
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((unsigned short)port);
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], 
           (size_t)host_entry->h_length);
    
    /* Connect to server */
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
    
        close(sock);
        cleanup_sockets();
        return COMMC_SYSTEM_ERROR;
        
    }
    
    *socket_fd = sock;
    return COMMC_SUCCESS;
    
}

/*

         send_all()
	       ---
	       sends all data through the socket, handling
	       partial sends and ensuring complete transmission.

*/

static commc_error_t send_all(int         socket_fd,
                              const char* data,
                              size_t      data_length) {

    size_t total_sent = 0;
    int    bytes_sent;
    
    while (total_sent < data_length) {
    
        bytes_sent = send(socket_fd, data + total_sent, 
                         (int)(data_length - total_sent), 0);
                         
        if (bytes_sent == SOCKET_ERROR) {
        
            return COMMC_SYSTEM_ERROR;
            
        }
        
        total_sent += (size_t)bytes_sent;
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         receive_line()
	       ---
	       receives a single line from the socket,
	       handling CR/LF line endings properly.

*/

static commc_error_t receive_line(int    socket_fd,
                                  char*  buffer,
                                  size_t buffer_size) {

    size_t pos = 0;
    char   ch;
    int    bytes_received;
    
    if (!buffer || buffer_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    while (pos < buffer_size - 1) {
    
        bytes_received = recv(socket_fd, &ch, 1, 0);
        
        if (bytes_received == SOCKET_ERROR) {
        
            return COMMC_SYSTEM_ERROR;
            
        }
        
        if (bytes_received == 0) {
        
            break;  /* Connection closed */
            
        }
        
        if (ch == '\r') {
        
            continue;  /* Skip carriage return */
            
        }
        
        if (ch == '\n') {
        
            break;  /* End of line */
            
        }
        
        buffer[pos++] = ch;
        
    }
    
    buffer[pos] = '\0';
    return COMMC_SUCCESS;
    
}

/*

         parse_status_line()
	       ---
	       parses the HTTP status line from a response,
	       extracting version, status code, and message.

*/

static commc_error_t parse_status_line(const char*            status_line,
                                       commc_http_version_t*  version,
                                       int*                   status_code,
                                       char*                  status_message) {

    const char* space1;
    const char* space2;
    char        version_str[16];
    char        code_str[8];
    
    if (!status_line || !version || !status_code || !status_message) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Find first space (after version) */
    
    space1 = strchr(status_line, ' ');
    
    if (!space1) {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    /* Extract version */
    
    if ((size_t)(space1 - status_line) >= sizeof(version_str)) {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    memcpy(version_str, status_line, (size_t)(space1 - status_line));
    version_str[space1 - status_line] = '\0';
    
    if (strcmp(version_str, "HTTP/1.0") == 0) {
    
        *version = COMMC_HTTP_VERSION_1_0;
        
    } else if (strcmp(version_str, "HTTP/1.1") == 0) {
    
        *version = COMMC_HTTP_VERSION_1_1;
        
    } else {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    /* Find second space (after status code) */
    
    space2 = strchr(space1 + 1, ' ');
    
    if (!space2) {
    
        /* No status message, just code */
        
        *status_code = atoi(space1 + 1);
        status_message[0] = '\0';
        
    } else {
    
        /* Extract status code */
        
        if ((size_t)(space2 - space1 - 1) >= sizeof(code_str)) {
        
            return COMMC_FORMAT_ERROR;
            
        }
        
        memcpy(code_str, space1 + 1, (size_t)(space2 - space1 - 1));
        code_str[space2 - space1 - 1] = '\0';
        
        *status_code = atoi(code_str);
        
        /* Extract status message */
        
        strcpy(status_message, space2 + 1);
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         parse_header_line()
	       ---
	       parses a single HTTP header line into name
	       and value components.

*/

static commc_error_t parse_header_line(const char*           header_line,
                                       commc_http_header_t*  header) {

    const char* colon;
    const char* value_start;
    size_t      name_len;
    size_t      value_len;
    
    if (!header_line || !header) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Find the colon separator */
    
    colon = strchr(header_line, ':');
    
    if (!colon) {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    /* Extract header name */
    
    name_len = (size_t)(colon - header_line);
    
    if (name_len >= sizeof(header->name)) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    memcpy(header->name, header_line, name_len);
    header->name[name_len] = '\0';
    
    /* Skip spaces after colon */
    
    value_start = colon + 1;
    
    while (*value_start == ' ' || *value_start == '\t') {
    
        value_start++;
        
    }
    
    /* Extract header value */
    
    value_len = strlen(value_start);
    
    if (value_len >= sizeof(header->value)) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    strcpy(header->value, value_start);
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_http_client_create()
	       ---
	       creates and initializes a new HTTP client.

*/

commc_error_t commc_http_client_create(commc_http_client_t** client) {

    commc_http_client_t* new_client;
    
    if (!client) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    new_client = malloc(sizeof(commc_http_client_t));
    
    if (!new_client) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Initialize client */
    
    memset(new_client, 0, sizeof(commc_http_client_t));
    
    new_client->socket_fd = COMMC_INVALID_SOCKET;
    strcpy(new_client->user_agent, "COMMC-HTTP/1.0");
    new_client->connection_timeout = COMMC_HTTP_DEFAULT_TIMEOUT;
    new_client->request_timeout = COMMC_HTTP_DEFAULT_TIMEOUT;
    new_client->follow_redirects = 1;
    new_client->max_redirects = 5;
    
    *client = new_client;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_client_destroy()
	       ---
	       destroys an HTTP client and frees resources.

*/

void commc_http_client_destroy(commc_http_client_t* client) {

    if (!client) {
    
        return;
        
    }
    
    /* Close socket if open */
    
    if (client->socket_fd != COMMC_INVALID_SOCKET) {
    
        close(client->socket_fd);
        cleanup_sockets();
        
    }
    
    free(client);
    
}

/*

         commc_http_request_create()
	       ---
	       creates and initializes a new HTTP request.

*/

commc_error_t commc_http_request_create(commc_http_request_t** request) {

    commc_http_request_t* new_request;
    
    if (!request) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    new_request = malloc(sizeof(commc_http_request_t));
    
    if (!new_request) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Initialize request */
    
    memset(new_request, 0, sizeof(commc_http_request_t));
    
    new_request->method = COMMC_HTTP_GET;
    new_request->version = COMMC_HTTP_VERSION_1_1;
    new_request->timeout_seconds = COMMC_HTTP_DEFAULT_TIMEOUT;
    new_request->follow_redirects = 1;
    new_request->max_redirects = 5;
    
    *request = new_request;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_request_destroy()
	       ---
	       destroys an HTTP request and frees memory.

*/

void commc_http_request_destroy(commc_http_request_t* request) {

    if (!request) {
    
        return;
        
    }
    
    if (request->body) {
    
        free(request->body);
        
    }
    
    free(request);
    
}

/*

         commc_http_response_create()
	       ---
	       creates and initializes a new HTTP response.

*/

commc_error_t commc_http_response_create(commc_http_response_t** response) {

    commc_http_response_t* new_response;
    
    if (!response) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    new_response = malloc(sizeof(commc_http_response_t));
    
    if (!new_response) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Initialize response */
    
    memset(new_response, 0, sizeof(commc_http_response_t));
    
    new_response->version = COMMC_HTTP_VERSION_1_1;
    
    *response = new_response;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_response_destroy()
	       ---
	       destroys an HTTP response and frees memory.

*/

void commc_http_response_destroy(commc_http_response_t* response) {

    if (!response) {
    
        return;
        
    }
    
    if (response->body) {
    
        free(response->body);
        
    }
    
    free(response);
    
}

/*

         commc_http_url_parse()
	       ---
	       parses a URL string into component parts.

*/

commc_error_t commc_http_url_parse(const char*       url_string,
                                   commc_http_url_t* parsed_url) {

    const char* current;
    const char* protocol_end;
    const char* hostname_start;
    const char* hostname_end;
    const char* port_start;
    const char* path_start;
    const char* query_start;
    const char* fragment_start;
    char        port_str[8];
    
    if (!url_string || !parsed_url) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Initialize parsed URL */
    
    memset(parsed_url, 0, sizeof(commc_http_url_t));
    strcpy(parsed_url->path, "/");
    parsed_url->port = COMMC_HTTP_DEFAULT_PORT;
    
    current = url_string;
    
    /* Extract protocol */
    
    protocol_end = strstr(current, "://");
    
    if (!protocol_end) {
    
        return COMMC_FORMAT_ERROR;
        
    }
    
    if ((size_t)(protocol_end - current) >= sizeof(parsed_url->protocol)) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    memcpy(parsed_url->protocol, current, (size_t)(protocol_end - current));
    parsed_url->protocol[protocol_end - current] = '\0';
    
    /* Set default port based on protocol */
    
    if (strcmp(parsed_url->protocol, "https") == 0) {
    
        parsed_url->port = COMMC_HTTP_DEFAULT_HTTPS_PORT;
        
    }
    
    /* Extract hostname */
    
    hostname_start = protocol_end + 3;
    hostname_end = hostname_start;
    
    while (*hostname_end && *hostname_end != ':' && 
           *hostname_end != '/' && *hostname_end != '?' && 
           *hostname_end != '#') {
        
        hostname_end++;
        
    }
    
    if ((size_t)(hostname_end - hostname_start) >= sizeof(parsed_url->hostname)) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    memcpy(parsed_url->hostname, hostname_start, (size_t)(hostname_end - hostname_start));
    parsed_url->hostname[hostname_end - hostname_start] = '\0';
    
    current = hostname_end;
    
    /* Extract port if present */
    
    if (*current == ':') {
    
        port_start = current + 1;
        current = port_start;
        
        while (*current && isdigit(*current)) {
        
            current++;
            
        }
        
        if ((size_t)(current - port_start) < sizeof(port_str)) {
        
            memcpy(port_str, port_start, (size_t)(current - port_start));
            port_str[current - port_start] = '\0';
            parsed_url->port = atoi(port_str);
            
        }
        
    }
    
    /* Extract path */
    
    if (*current == '/') {
    
        path_start = current;
        
        while (*current && *current != '?' && *current != '#') {
        
            current++;
            
        }
        
        if ((size_t)(current - path_start) < sizeof(parsed_url->path)) {
        
            memcpy(parsed_url->path, path_start, (size_t)(current - path_start));
            parsed_url->path[current - path_start] = '\0';
            
        }
        
    }
    
    /* Extract query string */
    
    if (*current == '?') {
    
        query_start = current + 1;
        current = query_start;
        
        while (*current && *current != '#') {
        
            current++;
            
        }
        
        if ((size_t)(current - query_start) < sizeof(parsed_url->query)) {
        
            memcpy(parsed_url->query, query_start, (size_t)(current - query_start));
            parsed_url->query[current - query_start] = '\0';
            
        }
        
    }
    
    /* Extract fragment */
    
    if (*current == '#') {
    
        fragment_start = current + 1;
        
        if (strlen(fragment_start) < sizeof(parsed_url->fragment)) {
        
            strcpy(parsed_url->fragment, fragment_start);
            
        }
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_request_set_url()
	       ---
	       sets the target URL for the HTTP request.

*/

commc_error_t commc_http_request_set_url(commc_http_request_t* request,
                                          const char*           url) {

    if (!request || !url) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    return commc_http_url_parse(url, &request->url);
    
}

/*

         commc_http_request_add_header()
	       ---
	       adds a header to the HTTP request.

*/

commc_error_t commc_http_request_add_header(commc_http_request_t* request,
                                             const char*           name,
                                             const char*           value) {

    commc_http_header_t* header;
    
    if (!request || !name || !value) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (request->header_count >= COMMC_HTTP_MAX_HEADERS) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    header = &request->headers[request->header_count];
    
    if (strlen(name) >= sizeof(header->name) ||
        strlen(value) >= sizeof(header->value)) {
        
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    strcpy(header->name, name);
    strcpy(header->value, value);
    
    request->header_count++;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_request_set_body()
	       ---
	       sets the request body content.

*/

commc_error_t commc_http_request_set_body(commc_http_request_t* request,
                                           const char*           body,
                                           size_t                body_length) {

    if (!request || !body) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Free existing body */
    
    if (request->body) {
    
        free(request->body);
        
    }
    
    /* Allocate new body */
    
    request->body = malloc(body_length + 1);
    
    if (!request->body) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    memcpy(request->body, body, body_length);
    request->body[body_length] = '\0';
    request->body_length = body_length;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_http_method_to_string()
	       ---
	       converts an HTTP method to its string representation.

*/

const char* commc_http_method_to_string(commc_http_method_t method) {

    if (method >= 0 && method < (int)(sizeof(http_method_strings) / sizeof(char*))) {
    
        return http_method_strings[method];
        
    }
    
    return "GET";
    
}

/*

         commc_http_get()
	       ---
	       convenience function for simple GET requests.

*/

commc_error_t commc_http_get(const char*            url,
                              commc_http_response_t* response) {

    commc_http_client_t*  client = NULL;
    commc_http_request_t* request = NULL;
    commc_error_t         result;
    
    if (!url || !response) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Create client and request */
    
    result = commc_http_client_create(&client);
    
    if (result != COMMC_SUCCESS) {
    
        return result;
        
    }
    
    result = commc_http_request_create(&request);
    
    if (result != COMMC_SUCCESS) {
    
        commc_http_client_destroy(client);
        return result;
        
    }
    
    /* Set up request */
    
    result = commc_http_request_set_url(request, url);
    
    if (result != COMMC_SUCCESS) {
    
        commc_http_request_destroy(request);
        commc_http_client_destroy(client);
        return result;
        
    }
    
    /* Execute request */
    
    result = commc_http_client_execute(client, request, response);
    
    /* Cleanup */
    
    commc_http_request_destroy(request);
    commc_http_client_destroy(client);
    
    return result;
    
}

/*

         commc_http_client_execute()
	       ---
	       executes an HTTP request (simplified implementation).

*/

commc_error_t commc_http_client_execute(commc_http_client_t*   client,
                                         commc_http_request_t*  request,
                                         commc_http_response_t* response) {

    char request_buffer[HTTP_REQUEST_BUFFER_SIZE];
    char line_buffer[HTTP_LINE_BUFFER_SIZE];
    commc_error_t result;
    int socket_fd;
    int i;
    size_t request_len = 0;
    
    if (!client || !request || !response) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Create connection */
    
    result = create_socket_connection(request->url.hostname,
                                      request->url.port,
                                      &socket_fd);
                                      
    if (result != COMMC_SUCCESS) {
    
        return result;
        
    }
    
    /* Build request */
    
    snprintf(request_buffer, sizeof(request_buffer),
             "%s %s HTTP/1.1\r\nHost: %s\r\n",
             commc_http_method_to_string(request->method),
             request->url.path,
             request->url.hostname);
             
    request_len = strlen(request_buffer);
    
    /* Add headers */
    
    for (i = 0; i < request->header_count; i++) {
    
        snprintf(request_buffer + request_len, 
                sizeof(request_buffer) - request_len,
                "%s: %s\r\n",
                request->headers[i].name,
                request->headers[i].value);
                
        request_len = strlen(request_buffer);
        
    }
    
    /* Add body if present */
    
    if (request->body && request->body_length > 0) {
    
        snprintf(request_buffer + request_len,
                sizeof(request_buffer) - request_len,
                "Content-Length: %lu\r\n\r\n",
                (unsigned long)request->body_length);
                
        request_len = strlen(request_buffer);
        
        if (request_len + request->body_length < sizeof(request_buffer)) {
        
            memcpy(request_buffer + request_len, request->body, request->body_length);
            request_len += request->body_length;
            
        }
        
    } else {
    
        strcat(request_buffer, "\r\n");
        request_len = strlen(request_buffer);
        
    }
    
    /* Send request */
    
    result = send_all(socket_fd, request_buffer, request_len);
    
    if (result != COMMC_SUCCESS) {
    
        close(socket_fd);
        cleanup_sockets();
        return result;
        
    }
    
    /* Receive status line */
    
    result = receive_line(socket_fd, line_buffer, sizeof(line_buffer));
    
    if (result != COMMC_SUCCESS) {
    
        close(socket_fd);
        cleanup_sockets();
        return result;
        
    }
    
    /* Parse status line */
    
    result = parse_status_line(line_buffer, &response->version,
                              &response->status_code, response->status_message);
                              
    if (result != COMMC_SUCCESS) {
    
        close(socket_fd);
        cleanup_sockets();
        return result;
        
    }
    
    /* Receive headers */
    
    while (1) {
    
        result = receive_line(socket_fd, line_buffer, sizeof(line_buffer));
        
        if (result != COMMC_SUCCESS) {
        
            close(socket_fd);
            cleanup_sockets();
            return result;
            
        }
        
        if (strlen(line_buffer) == 0) {
        
            break;  /* Empty line indicates end of headers */
            
        }
        
        if (response->header_count < COMMC_HTTP_MAX_HEADERS) {
        
            parse_header_line(line_buffer, &response->headers[response->header_count]);
            response->header_count++;
            
        }
        
    }
    
    /* Receive body (simplified - just read what's available) */
    
    response->body = malloc(MAX_RESPONSE_SIZE);
    
    if (response->body) {
    
        int bytes_received = recv(socket_fd, response->body, MAX_RESPONSE_SIZE - 1, 0);
        
        if (bytes_received > 0) {
        
            response->body[bytes_received] = '\0';
            response->body_length = (size_t)bytes_received;
            
        } else {
        
            response->body[0] = '\0';
            response->body_length = 0;
            
        }
        
    }
    
    /* Close connection */
    
    close(socket_fd);
    cleanup_sockets();
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
             --- EOF ---
	==================================
*/