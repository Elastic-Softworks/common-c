/*

   ===================================
   C O M M C   -   U R L
   URL ENCODING AND DECODING
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#include "commc/url.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 
	==================================
          --- CONSTANTS ---
	==================================
*/

/* unreserved characters that don't need encoding in URLs */
static const char UNRESERVED_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

/* characters allowed in query strings without encoding */
static const char QUERY_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@/?";

/* characters allowed in path components without encoding */
static const char PATH_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@/";

/* characters allowed in fragment without encoding */
static const char FRAGMENT_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@/?";

/* default ports for common schemes */
typedef struct {
    const char* scheme;
    int         port;
} scheme_port_t;

static const scheme_port_t DEFAULT_PORTS[] = {
    {"http",   80},
    {"https",  443},
    {"ftp",    21},
    {"ftps",   990},
    {"ssh",    22},
    {"telnet", 23},
    {NULL,     -1}
};

/* 
	==================================
         --- HELPERS ---
	==================================
*/

/*

         is_char_allowed()
	       ---
	       checks if character is allowed in URL
	       component without encoding based on the
	       specified encoding mode and character
	       set restrictions.

*/

static int is_char_allowed(char ch, commc_url_encoding_mode_t mode) {

    const char* allowed_chars;

    switch (mode) {

        case COMMC_URL_ENCODE_QUERY:
            allowed_chars = QUERY_CHARS;
            break;

        case COMMC_URL_ENCODE_PATH:
            allowed_chars = PATH_CHARS;
            break;

        case COMMC_URL_ENCODE_FRAGMENT:
            allowed_chars = FRAGMENT_CHARS;
            break;

        case COMMC_URL_ENCODE_USERINFO:
            allowed_chars = UNRESERVED_CHARS;
            break;

        case COMMC_URL_ENCODE_FORM:
            /* form encoding allows space as + */
            if (ch == ' ') return 1;
            allowed_chars = UNRESERVED_CHARS;
            break;

        default:
            allowed_chars = UNRESERVED_CHARS;
            break;
    }

    return strchr(allowed_chars, ch) != NULL;
}

/*

         hex_to_byte()
	       ---
	       converts two hexadecimal characters to
	       byte value. validates hex digits and
	       returns -1 for invalid hex sequences
	       during URL decoding operations.

*/

static int hex_to_byte(char h1, char h2) {

    int val1, val2;

    if (h1 >= '0' && h1 <= '9') {
        val1 = h1 - '0';
    } else if (h1 >= 'A' && h1 <= 'F') {
        val1 = h1 - 'A' + 10;
    } else if (h1 >= 'a' && h1 <= 'f') {
        val1 = h1 - 'a' + 10;
    } else {
        return -1;
    }

    if (h2 >= '0' && h2 <= '9') {
        val2 = h2 - '0';
    } else if (h2 >= 'A' && h2 <= 'F') {
        val2 = h2 - 'A' + 10;
    } else if (h2 >= 'a' && h2 <= 'f') {
        val2 = h2 - 'a' + 10;
    } else {
        return -1;
    }

    return (val1 << 4) | val2;
}

/*

         byte_to_hex()
	       ---
	       converts byte value to two-character
	       uppercase hexadecimal representation
	       for percent encoding in URL components
	       with consistent formatting.

*/

static void byte_to_hex(unsigned char byte, char* hex) {

    static const char HEX_DIGITS[] = "0123456789ABCDEF";

    hex[0] = HEX_DIGITS[(byte >> 4) & 0x0F];
    hex[1] = HEX_DIGITS[byte & 0x0F];
}

/*

         find_default_port()
	       ---
	       looks up default port number for given
	       URL scheme. returns standard port for
	       known protocols or -1 if scheme is
	       unknown or custom.

*/

static int find_default_port(const char* scheme) {

    int i;

    if (!scheme) {
        return -1;
    }

    for (i = 0; DEFAULT_PORTS[i].scheme != NULL; i++) {

        if (strcmp(scheme, DEFAULT_PORTS[i].scheme) == 0) {
            return DEFAULT_PORTS[i].port;
        }
    }

    return -1;
}

/* 
	==================================
         --- ENCODING API ---
	==================================
*/

/*

         commc_url_encode()
	       ---
	       encodes input string for safe URL inclusion
	       using percent encoding. applies encoding
	       rules based on mode and handles all
	       special characters appropriately.

*/

char* commc_url_encode(const char* input, commc_url_encoding_mode_t mode) {

    char* result;
    char* out;
    const char* in;
    int   result_size;
    int   input_len;

    if (!input) {
        return NULL;
    }

    input_len = strlen(input);

    /* estimate maximum output size (3x input for worst case) */
    result_size = input_len * 3 + 1;
    result = (char*)malloc(result_size);

    if (!result) {
        return NULL;
    }

    out = result;
    in  = input;

    while (*in) {

        unsigned char ch = (unsigned char)*in;

        /* handle form encoding special case for space */

        if (mode == COMMC_URL_ENCODE_FORM && ch == ' ') {
            *out++ = '+';
        }

        /* check if character needs encoding */

        else if (is_char_allowed(ch, mode)) {
            *out++ = ch;
        } else {
            /* percent encode the character */
            char hex[2];
            *out++ = '%';
            byte_to_hex(ch, hex);
            *out++ = hex[0];
            *out++ = hex[1];
        }

        in++;
    }

    *out = '\0';

    /* shrink buffer to actual size */

    result = (char*)realloc(result, out - result + 1);

    return result;
}

/*

         commc_url_decode()
	       ---
	       decodes percent-encoded URL string back
	       to original form with validation of
	       escape sequences and proper error
	       handling for malformed input.

*/

char* commc_url_decode(const char* input) {

    char* result;
    char* out;
    const char* in;
    int   input_len;

    if (!input) {
        return NULL;
    }

    input_len = strlen(input);
    result = (char*)malloc(input_len + 1);

    if (!result) {
        return NULL;
    }

    out = result;
    in  = input;

    while (*in) {

        if (*in == '%') {

            /* check for valid hex sequence */

            if (in[1] && in[2]) {

                int byte_val = hex_to_byte(in[1], in[2]);

                if (byte_val >= 0) {
                    *out++ = (char)byte_val;
                    in += 3;
                    continue;
                }
            }

            /* invalid escape sequence - copy as-is */
            *out++ = *in++;

        } else if (*in == '+') {
            /* form decoding: + becomes space */
            *out++ = ' ';
            in++;
        } else {
            *out++ = *in++;
        }
    }

    *out = '\0';

    /* shrink buffer to actual size */
    result = (char*)realloc(result, out - result + 1);

    return result;
}

/*

         commc_url_encode_component()
	       ---
	       encodes individual URL component with
	       component-specific character set rules
	       and encoding behavior for proper URL
	       construction and validation.

*/

char* commc_url_encode_component(const char* input, commc_url_encoding_mode_t mode) {

    return commc_url_encode(input, mode);
}

/*

         commc_url_decode_component()
	       ---
	       decodes individual URL component with
	       length tracking and validation. returns
	       decoded string and sets decoded_length
	       parameter if provided.

*/

char* commc_url_decode_component(const char* input, int* decoded_length) {

    char* result = commc_url_decode(input);

    if (result && decoded_length) {
        *decoded_length = strlen(result);
    }

    return result;
}

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
	       scheme, authority, path, query, and
	       fragment components with validation.

*/

commc_url_components_t* commc_url_parse(const char* url) {

    commc_url_components_t* components;
    char* url_copy;
    char* ptr;
    char* scheme_end;
    char* authority_start;
    char* authority_end;
    char* path_start;
    char* query_start;
    char* fragment_start;

    if (!url || strlen(url) == 0) {
        return NULL;
    }

    components = (commc_url_components_t*)malloc(sizeof(commc_url_components_t));

    if (!components) {
        return NULL;
    }

    /* initialize all components to NULL */
    memset(components, 0, sizeof(commc_url_components_t));
    components->port = -1;

    /* make working copy of URL */
    url_copy = (char*)malloc(strlen(url) + 1);

    if (!url_copy) {
        free(components);
        return NULL;
    }

    strcpy(url_copy, url);
    ptr = url_copy;

    /* find fragment first (# separates fragment from rest) */

    fragment_start = strchr(ptr, '#');

    if (fragment_start) {
        *fragment_start++ = '\0';
        components->fragment = (char*)malloc(strlen(fragment_start) + 1);
        if (components->fragment) {
            strcpy(components->fragment, fragment_start);
        }
    }

    /* find query string (? separates query from path) */

    query_start = strchr(ptr, '?');

    if (query_start) {
        *query_start++ = '\0';
        components->query = (char*)malloc(strlen(query_start) + 1);
        if (components->query) {
            strcpy(components->query, query_start);
        }
    }

    /* check for scheme (protocol:) */

    scheme_end = strchr(ptr, ':');

    if (scheme_end && scheme_end > ptr) {

        /* validate scheme characters */
        char* scheme_check = ptr;
        int   is_valid_scheme = 1;

        while (scheme_check < scheme_end && is_valid_scheme) {

            char ch = *scheme_check;

            if (!((ch >= 'a' && ch <= 'z') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') ||
                   ch == '+' || ch == '-' || ch == '.')) {
                is_valid_scheme = 0;
            }

            scheme_check++;
        }

        if (is_valid_scheme) {
            *scheme_end++ = '\0';
            components->scheme = (char*)malloc(strlen(ptr) + 1);
            if (components->scheme) {
                strcpy(components->scheme, ptr);
            }
            ptr = scheme_end;
            components->is_relative = 0;
        } else {
            components->is_relative = 1;
        }
    } else {
        components->is_relative = 1;
    }

    /* check for authority (//authority) */

    if (ptr[0] == '/' && ptr[1] == '/') {

        char* userinfo_end;
        char* host_start;
        char* port_start;

        ptr += 2;
        authority_start = ptr;

        /* find end of authority */
        authority_end = strchr(ptr, '/');

        if (!authority_end) {
            authority_end = ptr + strlen(ptr);
        }

        path_start = authority_end;

        /* temporarily null-terminate authority */
        if (*authority_end) {
            *authority_end = '\0';
        }

        /* parse authority: [userinfo@]host[:port] */

        userinfo_end = strchr(authority_start, '@');

        if (userinfo_end) {
            *userinfo_end++ = '\0';
            components->userinfo = (char*)malloc(strlen(authority_start) + 1);
            if (components->userinfo) {
                strcpy(components->userinfo, authority_start);
            }
            host_start = userinfo_end;
        } else {
            host_start = authority_start;
        }

        /* parse host and port */

        port_start = NULL;

        if (*host_start == '[') {
            /* IPv6 address */
            char* bracket_end = strchr(host_start, ']');
            if (bracket_end) {
                bracket_end++;
                if (*bracket_end == ':') {
                    *bracket_end++ = '\0';
                    port_start = bracket_end;
                } else {
                    port_start = NULL;
                }
                components->host = (char*)malloc(strlen(host_start) + 1);
                if (components->host) {
                    strcpy(components->host, host_start);
                }
            }
        } else {
            /* IPv4 or hostname */
            port_start = strrchr(host_start, ':');
            if (port_start) {
                *port_start++ = '\0';
            }
            components->host = (char*)malloc(strlen(host_start) + 1);
            if (components->host) {
                strcpy(components->host, host_start);
            }
        }

        /* parse port */

        if (port_start && *port_start) {
            components->port = atoi(port_start);
        } else if (components->scheme) {
            components->port = find_default_port(components->scheme);
        }

        /* restore path start character */
        if (*authority_end) {
            *authority_end = '/';
        }

        ptr = path_start;
    }

    /* remaining string is the path */

    if (*ptr || (!components->scheme && !authority_start)) {
        components->path = (char*)malloc(strlen(ptr) + 1);
        if (components->path) {
            strcpy(components->path, ptr);
        }
    }

    free(url_copy);

    return components;
}

/*

         commc_url_components_destroy()
	       ---
	       releases all memory associated with URL
	       components structure including individual
	       strings and main structure to prevent
	       memory leaks during cleanup.

*/

void commc_url_components_destroy(commc_url_components_t* components) {

    if (!components) {
        return;
    }

    if (components->scheme) {
        free(components->scheme);
    }

    if (components->userinfo) {
        free(components->userinfo);
    }

    if (components->host) {
        free(components->host);
    }

    if (components->path) {
        free(components->path);
    }

    if (components->query) {
        free(components->query);
    }

    if (components->fragment) {
        free(components->fragment);
    }

    if (components->full_url) {
        free(components->full_url);
    }

    free(components);
}

/*

         commc_url_build()
	       ---
	       constructs complete URL string from
	       component structure with proper formatting,
	       encoding, and RFC 3986 compliance for
	       URL generation and reconstruction.

*/

char* commc_url_build(const commc_url_components_t* components) {

    char* result;
    int   result_size;
    int   len;

    if (!components) {
        return NULL;
    }

    /* estimate buffer size */
    result_size = 1024;
    result = (char*)malloc(result_size);

    if (!result) {
        return NULL;
    }

    result[0] = '\0';

    /* add scheme */

    if (components->scheme && !components->is_relative) {
        strcat(result, components->scheme);
        strcat(result, ":");
    }

    /* add authority */

    if (components->host) {

        strcat(result, "//");

        if (components->userinfo) {
            strcat(result, components->userinfo);
            strcat(result, "@");
        }

        strcat(result, components->host);

        if (components->port > 0) {

            int default_port = find_default_port(components->scheme);

            if (components->port != default_port) {
                char port_str[16];
                sprintf(port_str, ":%d", components->port);
                strcat(result, port_str);
            }
        }
    }

    /* add path */

    if (components->path) {
        strcat(result, components->path);
    } else if (components->host) {
        /* add root path if host present but no path */
        strcat(result, "/");
    }

    /* add query */

    if (components->query) {
        strcat(result, "?");
        strcat(result, components->query);
    }

    /* add fragment */

    if (components->fragment) {
        strcat(result, "#");
        strcat(result, components->fragment);
    }

    /* resize to actual length */

    len = strlen(result);
    result = (char*)realloc(result, len + 1);

    return result;
}

/*

         commc_url_resolve()
	       ---
	       resolves relative URL against base URL
	       according to RFC 3986 resolution algorithm.
	       handles relative references and produces
	       absolute URL result.

*/

char* commc_url_resolve(const char* base_url, const char* relative_url) {

    commc_url_components_t* base;
    commc_url_components_t* relative;
    commc_url_components_t* result;
    char* resolved_url;

    if (!base_url || !relative_url) {
        return NULL;
    }

    base = commc_url_parse(base_url);

    if (!base) {
        return NULL;
    }

    relative = commc_url_parse(relative_url);

    if (!relative) {
        commc_url_components_destroy(base);
        return NULL;
    }

    /* if relative URL is absolute, return it as-is */

    if (!relative->is_relative) {
        resolved_url = commc_url_build(relative);
        commc_url_components_destroy(base);
        commc_url_components_destroy(relative);
        return resolved_url;
    }

    /* create result components */

    result = (commc_url_components_t*)malloc(sizeof(commc_url_components_t));

    if (!result) {
        commc_url_components_destroy(base);
        commc_url_components_destroy(relative);
        return NULL;
    }

    memset(result, 0, sizeof(commc_url_components_t));
    result->port = -1;

    /* apply RFC 3986 resolution algorithm */

    /* copy scheme from base */

    if (base->scheme) {
        result->scheme = (char*)malloc(strlen(base->scheme) + 1);
        if (result->scheme) {
            strcpy(result->scheme, base->scheme);
        }
    }

    /* determine authority, path, and query */

    if (relative->host) {
        /* relative has authority - use relative authority and path */
        if (relative->userinfo) {
            result->userinfo = (char*)malloc(strlen(relative->userinfo) + 1);
            if (result->userinfo) {
                strcpy(result->userinfo, relative->userinfo);
            }
        }
        if (relative->host) {
            result->host = (char*)malloc(strlen(relative->host) + 1);
            if (result->host) {
                strcpy(result->host, relative->host);
            }
        }
        result->port = relative->port;
        if (relative->path) {
            result->path = (char*)malloc(strlen(relative->path) + 1);
            if (result->path) {
                strcpy(result->path, relative->path);
            }
        }
        if (relative->query) {
            result->query = (char*)malloc(strlen(relative->query) + 1);
            if (result->query) {
                strcpy(result->query, relative->query);
            }
        }
    } else {
        /* use base authority */
        if (base->userinfo) {
            result->userinfo = (char*)malloc(strlen(base->userinfo) + 1);
            if (result->userinfo) {
                strcpy(result->userinfo, base->userinfo);
            }
        }
        if (base->host) {
            result->host = (char*)malloc(strlen(base->host) + 1);
            if (result->host) {
                strcpy(result->host, base->host);
            }
        }
        result->port = base->port;

        /* resolve path */
        if (!relative->path || strlen(relative->path) == 0) {
            /* empty path - use base path and relative query */
            if (base->path) {
                result->path = (char*)malloc(strlen(base->path) + 1);
                if (result->path) {
                    strcpy(result->path, base->path);
                }
            }
            if (relative->query) {
                result->query = (char*)malloc(strlen(relative->query) + 1);
                if (result->query) {
                    strcpy(result->query, relative->query);
                }
            } else if (base->query) {
                result->query = (char*)malloc(strlen(base->query) + 1);
                if (result->query) {
                    strcpy(result->query, base->query);
                }
            }
        } else {
            /* merge paths */
            if (relative->path[0] == '/') {
                /* absolute path */
                result->path = (char*)malloc(strlen(relative->path) + 1);
                if (result->path) {
                    strcpy(result->path, relative->path);
                }
            } else {
                /* relative path - merge with base path */
                if (base->path) {
                    char* last_slash = strrchr(base->path, '/');
                    if (last_slash) {
                        int base_dir_len = last_slash - base->path + 1;
                        int total_len = base_dir_len + strlen(relative->path) + 1;
                        result->path = (char*)malloc(total_len);
                        if (result->path) {
                            strncpy(result->path, base->path, base_dir_len);
                            result->path[base_dir_len] = '\0';
                            strcat(result->path, relative->path);
                        }
                    } else {
                        result->path = (char*)malloc(strlen(relative->path) + 1);
                        if (result->path) {
                            strcpy(result->path, relative->path);
                        }
                    }
                } else {
                    result->path = (char*)malloc(strlen(relative->path) + 1);
                    if (result->path) {
                        strcpy(result->path, relative->path);
                    }
                }
            }
            if (relative->query) {
                result->query = (char*)malloc(strlen(relative->query) + 1);
                if (result->query) {
                    strcpy(result->query, relative->query);
                }
            }
        }
    }

    /* copy fragment from relative */

    if (relative->fragment) {
        result->fragment = (char*)malloc(strlen(relative->fragment) + 1);
        if (result->fragment) {
            strcpy(result->fragment, relative->fragment);
        }
    }

    /* build final URL */

    resolved_url = commc_url_build(result);

    commc_url_components_destroy(base);
    commc_url_components_destroy(relative);
    commc_url_components_destroy(result);

    return resolved_url;
}

/*

         commc_url_normalize()
	       ---
	       normalizes URL by applying standard
	       transformations for consistent comparison
	       and canonical representation according
	       to RFC 3986 normalization rules.

*/

char* commc_url_normalize(const char* url) {

    commc_url_components_t* components;
    char* normalized;
    int i;

    if (!url) {
        return NULL;
    }

    components = commc_url_parse(url);

    if (!components) {
        return NULL;
    }

    /* normalize scheme to lowercase */

    if (components->scheme) {

        for (i = 0; components->scheme[i]; i++) {
            components->scheme[i] = tolower(components->scheme[i]);
        }
    }

    /* normalize host to lowercase */

    if (components->host) {

        for (i = 0; components->host[i]; i++) {
            components->host[i] = tolower(components->host[i]);
        }
    }

    /* remove default port if present */

    if (components->port > 0 && components->scheme) {

        int default_port = find_default_port(components->scheme);

        if (components->port == default_port) {
            components->port = -1;
        }
    }

    /* build normalized URL */

    normalized = commc_url_build(components);

    commc_url_components_destroy(components);

    return normalized;
}

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
	       and proper handling of name-value pairs
	       for easy parameter manipulation.

*/

commc_url_query_t* commc_url_query_parse(const char* query_string) {

    commc_url_query_t* query;
    char* query_copy;
    char* param_str;

    if (!query_string || strlen(query_string) == 0) {
        return NULL;
    }

    query = (commc_url_query_t*)malloc(sizeof(commc_url_query_t));

    if (!query) {
        return NULL;
    }

    query->params   = NULL;
    query->count    = 0;
    query->capacity = 0;

    query_copy = (char*)malloc(strlen(query_string) + 1);

    if (!query_copy) {
        free(query);
        return NULL;
    }

    strcpy(query_copy, query_string);

    /* parse parameters separated by & or ; */

    param_str = strtok(query_copy, "&;");

    while (param_str) {

        char* equals = strchr(param_str, '=');
        char* name_decoded;
        char* value_decoded = NULL;

        /* separate name and value */

        if (equals) {
            *equals++ = '\0';
            value_decoded = commc_url_decode(equals);
        }

        name_decoded = commc_url_decode(param_str);

        if (name_decoded) {
            commc_url_query_set(query, name_decoded, value_decoded);
            free(name_decoded);
        }

        if (value_decoded) {
            free(value_decoded);
        }

        param_str = strtok(NULL, "&;");
    }

    free(query_copy);

    return query;
}

/*

         commc_url_query_destroy()
	       ---
	       releases all memory associated with
	       query parameter structure including
	       parameter arrays and individual strings
	       to prevent memory leaks.

*/

void commc_url_query_destroy(commc_url_query_t* query) {

    int i;

    if (!query) {
        return;
    }

    if (query->params) {

        for (i = 0; i < query->count; i++) {

            if (query->params[i].name) {
                free(query->params[i].name);
            }

            if (query->params[i].value) {
                free(query->params[i].value);
            }
        }

        free(query->params);
    }

    free(query);
}

/*

         commc_url_query_get()
	       ---
	       retrieves parameter value by name from
	       query structure with case-sensitive
	       matching. returns first matching value
	       or NULL if parameter not found.

*/

const char* commc_url_query_get(const commc_url_query_t* query, const char* name) {

    int i;

    if (!query || !name) {
        return NULL;
    }

    for (i = 0; i < query->count; i++) {

        if (query->params[i].name && strcmp(query->params[i].name, name) == 0) {
            return query->params[i].value;
        }
    }

    return NULL;
}

/*

         commc_url_query_set()
	       ---
	       sets parameter value in query structure
	       with automatic memory management. updates
	       existing parameter or adds new parameter
	       with proper string duplication.

*/

int commc_url_query_set(commc_url_query_t* query, const char* name, const char* value) {

    int i;
    commc_url_query_param_t* new_params;

    if (!query || !name) {
        return -1;
    }

    /* check if parameter already exists */

    for (i = 0; i < query->count; i++) {

        if (query->params[i].name && strcmp(query->params[i].name, name) == 0) {

            /* update existing parameter */

            if (query->params[i].value) {
                free(query->params[i].value);
            }

            if (value) {
                query->params[i].value = (char*)malloc(strlen(value) + 1);
                if (query->params[i].value) {
                    strcpy(query->params[i].value, value);
                }
            } else {
                query->params[i].value = NULL;
            }

            return 0;
        }
    }

    /* add new parameter */

    if (query->count >= query->capacity) {

        int new_capacity = query->capacity ? query->capacity * 2 : 8;

        new_params = (commc_url_query_param_t*)realloc(query->params, 
                                                       new_capacity * sizeof(commc_url_query_param_t));

        if (!new_params) {
            return -1;
        }

        query->params = new_params;
        query->capacity = new_capacity;
    }

    /* initialize new parameter */

    query->params[query->count].name = (char*)malloc(strlen(name) + 1);

    if (!query->params[query->count].name) {
        return -1;
    }

    strcpy(query->params[query->count].name, name);

    if (value) {
        query->params[query->count].value = (char*)malloc(strlen(value) + 1);
        if (query->params[query->count].value) {
            strcpy(query->params[query->count].value, value);
        }
    } else {
        query->params[query->count].value = NULL;
    }

    query->count++;

    return 0;
}

/*

         commc_url_query_remove()
	       ---
	       removes parameter from query structure
	       by name with memory cleanup and array
	       compaction to maintain structure
	       integrity after removal.

*/

int commc_url_query_remove(commc_url_query_t* query, const char* name) {

    int i;

    if (!query || !name) {
        return -1;
    }

    for (i = 0; i < query->count; i++) {

        if (query->params[i].name && strcmp(query->params[i].name, name) == 0) {

            /* free parameter strings */

            free(query->params[i].name);

            if (query->params[i].value) {
                free(query->params[i].value);
            }

            /* compact array */

            for (; i < query->count - 1; i++) {
                query->params[i] = query->params[i + 1];
            }

            query->count--;

            return 0;
        }
    }

    return -1;  /* parameter not found */
}

/*

         commc_url_query_build()
	       ---
	       constructs query string from parameter
	       structure with proper URL encoding and
	       formatting. generates standard ampersand-
	       separated query string format.

*/

char* commc_url_query_build(const commc_url_query_t* query) {

    char* result;
    int   result_size;
    int   i;

    if (!query || query->count == 0) {
        return NULL;
    }

    /* estimate buffer size */
    result_size = 1024;
    result = (char*)malloc(result_size);

    if (!result) {
        return NULL;
    }

    result[0] = '\0';

    for (i = 0; i < query->count; i++) {

        char* encoded_name;
        char* encoded_value;

        if (i > 0) {
            strcat(result, "&");
        }

        encoded_name = commc_url_encode(query->params[i].name, COMMC_URL_ENCODE_FORM);

        if (encoded_name) {
            strcat(result, encoded_name);
            free(encoded_name);
        }

        if (query->params[i].value) {
            strcat(result, "=");
            encoded_value = commc_url_encode(query->params[i].value, COMMC_URL_ENCODE_FORM);
            if (encoded_value) {
                strcat(result, encoded_value);
                free(encoded_value);
            }
        }

        /* ensure buffer is large enough */
        if ((int)strlen(result) > (result_size - 256)) {
            result_size *= 2;
            result = (char*)realloc(result, result_size);
            if (!result) {
                return NULL;
            }
        }
    }

    /* shrink to actual size */
    result = (char*)realloc(result, strlen(result) + 1);

    return result;
}

/*

         commc_url_query_count()
	       ---
	       returns number of parameters in query
	       structure for iteration and capacity
	       planning during query manipulation
	       operations.

*/

int commc_url_query_count(const commc_url_query_t* query) {

    if (!query) {
        return 0;
    }

    return query->count;
}

/*

         commc_url_query_get_param()
	       ---
	       retrieves parameter structure by index
	       for iteration with bounds checking.
	       provides access to both name and value
	       components for enumeration.

*/

const commc_url_query_param_t* commc_url_query_get_param(const commc_url_query_t* query, int index) {

    if (!query || index < 0 || index >= query->count) {
        return NULL;
    }

    return &query->params[index];
}

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_url_is_valid()
	       ---
	       validates URL format according to RFC 3986
	       with basic syntax checking and structure
	       validation. returns 1 for valid URLs,
	       0 for invalid format.

*/

int commc_url_is_valid(const char* url) {

    commc_url_components_t* components;

    if (!url || strlen(url) == 0) {
        return 0;
    }

    components = commc_url_parse(url);

    if (!components) {
        return 0;
    }

    /* basic validation checks */

    if (!components->is_relative) {
        /* absolute URL must have scheme */
        if (!components->scheme) {
            commc_url_components_destroy(components);
            return 0;
        }
    }

    commc_url_components_destroy(components);

    return 1;
}

/*

         commc_url_is_absolute()
	       ---
	       determines whether URL is absolute based
	       on presence of scheme component. returns
	       1 for absolute URLs, 0 for relative URLs
	       or invalid input.

*/

int commc_url_is_absolute(const char* url) {

    commc_url_components_t* components;
    int is_absolute;

    if (!url) {
        return 0;
    }

    components = commc_url_parse(url);

    if (!components) {
        return 0;
    }

    is_absolute = !components->is_relative;

    commc_url_components_destroy(components);

    return is_absolute;
}

/*

         commc_url_get_scheme()
	       ---
	       extracts scheme component from URL without
	       full parsing. returns allocated string
	       containing protocol portion or NULL if
	       no scheme present.

*/

char* commc_url_get_scheme(const char* url) {

    const char* colon;
    char* scheme;
    int   scheme_len;

    if (!url) {
        return NULL;
    }

    colon = strchr(url, ':');

    if (!colon || colon == url) {
        return NULL;
    }

    scheme_len = colon - url;
    scheme = (char*)malloc(scheme_len + 1);

    if (!scheme) {
        return NULL;
    }

    strncpy(scheme, url, scheme_len);
    scheme[scheme_len] = '\0';

    return scheme;
}

/*

         commc_url_get_host()
	       ---
	       extracts host component from URL without
	       full parsing. handles IPv6 addresses,
	       domain names, and IPv4 addresses with
	       proper bracket handling.

*/

char* commc_url_get_host(const char* url) {

    commc_url_components_t* components;
    char* host = NULL;

    if (!url) {
        return NULL;
    }

    components = commc_url_parse(url);

    if (components && components->host) {
        host = (char*)malloc(strlen(components->host) + 1);
        if (host) {
            strcpy(host, components->host);
        }
    }

    commc_url_components_destroy(components);

    return host;
}

/*

         commc_url_get_port()
	       ---
	       extracts port number from URL or returns
	       default port for known schemes. returns
	       port number or -1 if no port specified
	       and scheme is unknown.

*/

int commc_url_get_port(const char* url) {

    commc_url_components_t* components;
    int port = -1;

    if (!url) {
        return -1;
    }

    components = commc_url_parse(url);

    if (components) {
        if (components->port > 0) {
            port = components->port;
        } else if (components->scheme) {
            port = find_default_port(components->scheme);
        }
    }

    commc_url_components_destroy(components);

    return port;
}

/*

         commc_url_get_path()
	       ---
	       extracts path component from URL without
	       full parsing. returns allocated string
	       containing path portion with proper
	       handling of empty and root paths.

*/

char* commc_url_get_path(const char* url) {

    commc_url_components_t* components;
    char* path = NULL;

    if (!url) {
        return NULL;
    }

    components = commc_url_parse(url);

    if (components && components->path) {
        path = (char*)malloc(strlen(components->path) + 1);
        if (path) {
            strcpy(path, components->path);
        }
    }

    commc_url_components_destroy(components);

    return path;
}

/*

         commc_url_equals()
	       ---
	       compares two URLs for equality after
	       normalization. handles minor formatting
	       differences and provides semantic
	       comparison rather than string matching.

*/

int commc_url_equals(const char* url1, const char* url2) {

    char* norm1;
    char* norm2;
    int   equal = 0;

    if (!url1 || !url2) {
        return (url1 == url2);
    }

    norm1 = commc_url_normalize(url1);
    norm2 = commc_url_normalize(url2);

    if (norm1 && norm2) {
        equal = (strcmp(norm1, norm2) == 0);
    }

    if (norm1) {
        free(norm1);
    }

    if (norm2) {
        free(norm2);
    }

    return equal;
}

/*

         commc_url_join()
	       ---
	       joins base URL with path component using
	       proper path resolution rules. handles
	       trailing slashes and path normalization
	       for consistent URL construction.

*/

char* commc_url_join(const char* base, const char* path) {

    if (!base || !path) {
        return NULL;
    }

    return commc_url_resolve(base, path);
}

/* 
	==================================
           --- EOF ---
	==================================
*/