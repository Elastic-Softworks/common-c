/* 	
   ===================================
   P A T H . C
   CROSS-PLATFORM PATH UTILITIES
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module implements comprehensive cross-platform
	    path manipulation utilities with proper normalization,
	    validation, and conversion capabilities.
	       
	    provides robust path handling for Windows and Unix
	    systems while maintaining C89 compliance and
	    educational documentation standards.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#include "commc/path.h"
#include "commc/error.h"

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#ifdef _WIN32
#define PATH_SEPARATOR      '\\'
#define PATH_SEPARATOR_STR  "\\"
#define ALT_SEPARATOR       '/'
#else
#define PATH_SEPARATOR      '/'
#define PATH_SEPARATOR_STR  "/"
#define ALT_SEPARATOR       '\0'
#endif

#define MAX_DRIVE_LEN       3      /* C:\ */
#define DOT_COMPONENT       "."
#define DOTDOT_COMPONENT    ".."

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         is_separator()
	       ---
	       checks if a character is a valid path separator.
	       handles both forward slash and backslash on Windows
	       for maximum compatibility.

*/

static int is_separator(char c) {

    return (c == PATH_SEPARATOR) || (c == ALT_SEPARATOR && ALT_SEPARATOR != '\0');
    
}

/*

         is_drive_letter()
	       ---
	       determines if a character is a valid drive letter.
	       Windows drive letters are A-Z or a-z.

*/

static int is_drive_letter(char c) {

    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    
}

/*

         get_format_separator()
	       ---
	       returns the appropriate separator character
	       for the specified path format.

*/

static char get_format_separator(commc_path_format_t format) {

    switch (format) {
    
        case COMMC_PATH_FORMAT_WINDOWS:
            return '\\';
            
        case COMMC_PATH_FORMAT_UNIX:
        case COMMC_PATH_FORMAT_URI:
            return '/';
            
        case COMMC_PATH_FORMAT_NATIVE:
        default:
            return PATH_SEPARATOR;
            
    }
    
}

/*

         skip_separators()
	       ---
	       advances a string pointer past consecutive
	       path separators, handling multiple separators
	       that should be treated as one.

*/

static const char* skip_separators(const char* path) {

    while (*path && is_separator(*path)) {
    
        path++;
        
    }
    
    return path;
    
}

/*

         find_next_separator()
	       ---
	       finds the next path separator in a string,
	       returning a pointer to the separator or
	       the end of the string.

*/

static const char* find_next_separator(const char* path) {

    while (*path && !is_separator(*path)) {
    
        path++;
        
    }
    
    return path;
    
}

/*

         is_valid_filename_char()
	       ---
	       checks if a character is valid in a filename
	       based on platform restrictions and reserved
	       characters.

*/

static int is_valid_filename_char(char c) {

#ifdef _WIN32

    /* Windows has more restricted filename characters */
    
    return !(c < 32 || c == '<' || c == '>' || c == ':' || 
             c == '"' || c == '|' || c == '?' || c == '*');
             
#else

    /* Unix allows most characters except null and slash */
    
    return c != '\0' && c != '/';
    
#endif

}

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_path_normalize()
	       ---
	       normalizes a file system path by resolving relative
	       components, removing redundant separators, and
	       converting to the specified format.

*/

commc_error_t commc_path_normalize(const char*         input_path,
                                   char*               output_path,
                                   size_t              output_size,
                                   commc_path_format_t format) {

    char*       components[COMMC_PATH_MAX_COMPONENTS];
    int         component_count = 0;
    const char* current;
    const char* next;
    char        temp_buffer[COMMC_PATH_MAX_LENGTH];
    char        separator;
    int         is_absolute_path;
    int         has_drive = 0;
    char        drive[4] = {0};
    size_t      output_pos = 0;
    int         i;
    
    if (!input_path || !output_path || output_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (strlen(input_path) >= COMMC_PATH_MAX_LENGTH) {
    
        return COMMC_IO_ERROR;
        
    }
    
    separator = get_format_separator(format);
    
    /* Copy input to temp buffer for processing */
    
    strcpy(temp_buffer, input_path);
    current = temp_buffer;
    
    /* Check if path is absolute and extract drive on Windows */
    
#ifdef _WIN32

    if (is_drive_letter(current[0]) && current[1] == ':') {
    
        drive[0] = current[0];
        drive[1] = current[1];
        drive[2] = '\0';
        has_drive = 1;
        current += 2;
        
    }
    
#endif

    /* Determine if path is absolute */
    
    is_absolute_path = is_separator(*current) || has_drive;
    
    if (is_separator(*current)) {
    
        current = skip_separators(current);
        
    }
    
    /* Split path into components and resolve . and .. */
    
    while (*current) {
    
        next = find_next_separator(current);
        
        if (next == current) {
        
            /* Empty component, skip */
            
            current = skip_separators(next);
            continue;
            
        }
        
        /* Check component length */
        
        if ((size_t)(next - current) >= sizeof(temp_buffer)) {
        
            return COMMC_IO_ERROR;
            
        }
        
        /* Handle . component */
        
        if (next - current == 1 && *current == '.') {
        
            current = skip_separators(next);
            continue;
            
        }
        
        /* Handle .. component */
        
        if (next - current == 2 && current[0] == '.' && current[1] == '.') {
        
            if (component_count > 0 && 
                strcmp(components[component_count - 1], "..") != 0) {
                
                /* Remove previous component */
                
                free(components[component_count - 1]);
                component_count--;
                
            } else if (!is_absolute_path) {
            
                /* Keep .. for relative paths */
                
                components[component_count] = malloc(3);
                
                if (!components[component_count]) {
                
                    /* Clean up allocated components */
                    
                    for (i = 0; i < component_count; i++) {
                    
                        free(components[i]);
                        
                    }
                    
                    return COMMC_MEMORY_ERROR;
                    
                }
                
                strcpy(components[component_count], "..");
                component_count++;
                
            }
            
            current = skip_separators(next);
            continue;
            
        }
        
        /* Regular component */
        
        if (component_count >= COMMC_PATH_MAX_COMPONENTS) {
        
            /* Clean up allocated components */
            
            for (i = 0; i < component_count; i++) {
            
                free(components[i]);
                
            }
            
            return COMMC_IO_ERROR;
            
        }
        
        components[component_count] = malloc((size_t)(next - current) + 1);
        
        if (!components[component_count]) {
        
            /* Clean up allocated components */
            
            for (i = 0; i < component_count; i++) {
            
                free(components[i]);
                
            }
            
            return COMMC_MEMORY_ERROR;
            
        }
        
        memcpy(components[component_count], current, (size_t)(next - current));
        components[component_count][next - current] = '\0';
        component_count++;
        
        current = skip_separators(next);
        
    }
    
    /* Rebuild normalized path */
    
    output_path[0] = '\0';
    output_pos = 0;
    
    /* Add drive on Windows */
    
#ifdef _WIN32

    if (has_drive) {
    
        if (output_pos + 2 >= output_size) {
        
            /* Clean up allocated components */
            
            for (i = 0; i < component_count; i++) {
            
                free(components[i]);
                
            }
            
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(output_path + output_pos, drive);
        output_pos += 2;
        
    }
    
#endif

    /* Add root separator for absolute paths */
    
    if (is_absolute_path) {
    
        if (output_pos + 1 >= output_size) {
        
            /* Clean up allocated components */
            
            for (i = 0; i < component_count; i++) {
            
                free(components[i]);
                
            }
            
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        output_path[output_pos++] = separator;
        output_path[output_pos] = '\0';
        
    }
    
    /* Add components */
    
    for (i = 0; i < component_count; i++) {
    
        size_t comp_len = strlen(components[i]);
        
        if (i > 0) {
        
            if (output_pos + 1 >= output_size) {
            
                /* Clean up remaining components */
                
                while (i < component_count) {
                
                    free(components[i]);
                    i++;
                    
                }
                
                return COMMC_ERROR_BUFFER_TOO_SMALL;
                
            }
            
            output_path[output_pos++] = separator;
            
        }
        
        if (output_pos + comp_len >= output_size) {
        
            /* Clean up remaining components */
            
            while (i < component_count) {
            
                free(components[i]);
                i++;
                
            }
            
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(output_path + output_pos, components[i]);
        output_pos += comp_len;
        
        free(components[i]);
        
    }
    
    /* Handle empty path case */
    
    if (output_pos == 0 || (is_absolute_path && output_pos == 1)) {
    
        if (!is_absolute_path) {
        
            if (output_size < 2) {
            
                return COMMC_ERROR_BUFFER_TOO_SMALL;
                
            }
            
            strcpy(output_path, ".");
            
        }
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         commc_path_join()
	       ---
	       joins multiple path components into a single path
	       with proper separator handling.

*/

commc_error_t commc_path_join(char*               output_path,
                              size_t              output_size,
                              commc_path_format_t format,
                              int                 component_count,
                              ...) {

    va_list     args;
    const char* component;
    char        separator;
    size_t      output_pos = 0;
    int         i;
    size_t      comp_len;
    int         need_separator;
    
    if (!output_path || output_size == 0 || component_count <= 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    separator = get_format_separator(format);
    output_path[0] = '\0';
    
    va_start(args, component_count);
    
    for (i = 0; i < component_count; i++) {
    
        component = va_arg(args, const char*);
        
        if (!component) {
        
            continue;
            
        }
        
        comp_len = strlen(component);
        
        if (comp_len == 0) {
        
            continue;
            
        }
        
        /* Determine if we need a separator */
        
        need_separator = (output_pos > 0 && 
                         output_path[output_pos - 1] != separator &&
                         !is_separator(output_path[output_pos - 1]) &&
                         !is_separator(component[0]));
        
        /* Add separator if needed */
        
        if (need_separator) {
        
            if (output_pos + 1 >= output_size) {
            
                va_end(args);
                return COMMC_ERROR_BUFFER_TOO_SMALL;
                
            }
            
            output_path[output_pos++] = separator;
            
        }
        
        /* Add component */
        
        if (output_pos + comp_len >= output_size) {
        
            va_end(args);
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(output_path + output_pos, component);
        output_pos += comp_len;
        
    }
    
    va_end(args);
    
    return COMMC_SUCCESS;
    
}

/*

         commc_path_split()
	       ---
	       splits a path into its component parts.

*/

commc_error_t commc_path_split(const char*        path,
                               commc_path_info_t* info) {

    const char* current;
    const char* last_separator;
    const char* extension_pos;
    const char* filename_start;
    size_t      path_len;
    int         component_index = 0;
    const char* comp_start;
    const char* comp_end;
    
    if (!path || !info) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Initialize info structure */
    
    memset(info, 0, sizeof(commc_path_info_t));
    path_len = strlen(path);
    
    if (path_len == 0) {
    
        info->type = COMMC_PATH_RELATIVE;
        info->format = COMMC_PATH_FORMAT_NATIVE;
        return COMMC_SUCCESS;
        
    }
    
    current = path;
    
    /* Detect format based on separators */
    
    if (strchr(path, '\\')) {
    
        info->format = COMMC_PATH_FORMAT_WINDOWS;
        
    } else {
    
        info->format = COMMC_PATH_FORMAT_UNIX;
        
    }
    
    /* Extract drive on Windows */
    
#ifdef _WIN32

    if (is_drive_letter(current[0]) && current[1] == ':') {
    
        info->drive[0] = current[0];
        info->drive[1] = current[1];
        info->drive[2] = '\0';
        current += 2;
        
    }
    
#endif

    /* Determine path type */
    
    if (strlen(info->drive) > 0) {
    
        info->type = COMMC_PATH_ABSOLUTE;
        
    } else if (is_separator(*current)) {
    
        info->type = COMMC_PATH_ABSOLUTE;
        
    } else if (strncmp(current, "./", 2) == 0 || strncmp(current, ".\\", 2) == 0) {
    
        info->type = COMMC_PATH_CURRENT;
        
    } else {
    
        info->type = COMMC_PATH_RELATIVE;
        
    }
    
    /* Skip initial separators */
    
    if (is_separator(*current)) {
    
        current = skip_separators(current);
        
    }
    
    /* Find last separator to split directory and filename */
    
    last_separator = NULL;
    filename_start = current;
    
    while (*current) {
    
        if (is_separator(*current)) {
        
            last_separator = current;
            filename_start = current + 1;
            
        }
        
        current++;
        
    }
    
    /* Extract directory portion */
    
    if (last_separator) {
    
        size_t dir_len = (size_t)(last_separator - path);
        
        if (dir_len >= sizeof(info->directory)) {
        
            return COMMC_IO_ERROR;
            
        }
        
        memcpy(info->directory, path, dir_len);
        info->directory[dir_len] = '\0';
        
    } else {
    
        /* No directory separator found */
        
        if (strlen(info->drive) > 0) {
        
            strcpy(info->directory, info->drive);
            
        } else {
        
            info->directory[0] = '\0';
            
        }
        
    }
    
    /* Extract filename and extension */
    
    if (*filename_start) {
    
        if (strlen(filename_start) >= sizeof(info->filename)) {
        
            return COMMC_IO_ERROR;
            
        }
        
        strcpy(info->filename, filename_start);
        
        /* Find extension */
        
        extension_pos = strrchr(info->filename, '.');
        
        if (extension_pos && extension_pos != info->filename) {
        
            strcpy(info->extension, extension_pos);
            info->has_extension = 1;
            
        }
        
    }
    
    /* Check if path represents a directory */
    
    info->is_directory = (path_len > 0 && is_separator(path[path_len - 1])) ||
                        (info->filename[0] == '\0');
    
    /* Split into components */
    
    current = path;
    
    /* Skip drive */
    
    if (strlen(info->drive) > 0) {
    
        current += strlen(info->drive);
        
    }
    
    /* Skip initial separators */
    
    current = skip_separators(current);
    
    /* Extract components */
    
    while (*current && component_index < COMMC_PATH_MAX_COMPONENTS) {
    
        comp_start = current;
        comp_end = find_next_separator(current);
        
        if (comp_end > comp_start) {
        
            size_t comp_len = (size_t)(comp_end - comp_start);
            
            info->components[component_index] = malloc(comp_len + 1);
            
            if (!info->components[component_index]) {
            
                /* Clean up previously allocated components */
                
                while (component_index > 0) {
                
                    component_index--;
                    free(info->components[component_index]);
                    
                }
                
                return COMMC_MEMORY_ERROR;
                
            }
            
            memcpy(info->components[component_index], comp_start, comp_len);
            info->components[component_index][comp_len] = '\0';
            component_index++;
            
        }
        
        current = skip_separators(comp_end);
        
    }
    
    info->component_count = component_index;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_path_basename()
	       ---
	       extracts the filename portion from a path.

*/

commc_error_t commc_path_basename(const char* path,
                                  char*       basename,
                                  size_t      basename_size,
                                  int         strip_extension) {

    const char* filename_start;
    const char* current;
    const char* extension_pos = NULL;
    size_t      copy_len;
    
    if (!path || !basename || basename_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Find the last separator */
    
    filename_start = path;
    current = path;
    
    while (*current) {
    
        if (is_separator(*current)) {
        
            filename_start = current + 1;
            
        }
        
        current++;
        
    }
    
    /* Handle empty filename case */
    
    if (*filename_start == '\0') {
    
        if (basename_size < 2) {
        
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(basename, ".");
        return COMMC_SUCCESS;
        
    }
    
    /* Find extension if we need to strip it */
    
    if (strip_extension) {
    
        extension_pos = strrchr(filename_start, '.');
        
        if (extension_pos == filename_start) {
        
            extension_pos = NULL;  /* Don't strip if filename starts with dot */
            
        }
        
    }
    
    /* Calculate copy length */
    
    if (extension_pos) {
    
        copy_len = (size_t)(extension_pos - filename_start);
        
    } else {
    
        copy_len = strlen(filename_start);
        
    }
    
    /* Check buffer size */
    
    if (copy_len >= basename_size) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    /* Copy filename */
    
    memcpy(basename, filename_start, copy_len);
    basename[copy_len] = '\0';
    
    return COMMC_SUCCESS;
    
}

/*

         commc_path_dirname()
	       ---
	       extracts the directory portion from a path.

*/

commc_error_t commc_path_dirname(const char* path,
                                 char*       dirname,
                                 size_t      dirname_size) {

    const char* last_separator = NULL;
    const char* current = path;
    size_t      copy_len;
    
    if (!path || !dirname || dirname_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Find the last separator */
    
    while (*current) {
    
        if (is_separator(*current)) {
        
            last_separator = current;
            
        }
        
        current++;
        
    }
    
    /* Handle cases with no separator */
    
    if (!last_separator) {
    
        if (dirname_size < 2) {
        
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(dirname, ".");
        return COMMC_SUCCESS;
        
    }
    
    /* Calculate directory length */
    
    copy_len = (size_t)(last_separator - path);
    
    /* Handle root directory case */
    
    if (copy_len == 0) {
    
        copy_len = 1;  /* Include the root separator */
        
    }
    
    /* Check buffer size */
    
    if (copy_len >= dirname_size) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    /* Copy directory portion */
    
    memcpy(dirname, path, copy_len);
    dirname[copy_len] = '\0';
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
           --- VALIDATION ---
	==================================
*/

/*

         commc_path_is_absolute()
	       ---
	       determines whether a path is absolute or relative.

*/

int commc_path_is_absolute(const char* path) {

    if (!path || *path == '\0') {
    
        return 0;
        
    }
    
#ifdef _WIN32

    /* Windows: Check for drive letter or UNC path */
    
    if (is_drive_letter(path[0]) && path[1] == ':') {
    
        return 1;
        
    }
    
    if (path[0] == '\\' && path[1] == '\\') {
    
        return 1;  /* UNC path */
        
    }
    
#endif

    /* Unix-style absolute path */
    
    return is_separator(path[0]);
    
}

/*

         commc_path_is_valid()
	       ---
	       validates a path string for proper format.

*/

int commc_path_is_valid(const char* path) {

    const char* current;
    
    if (!path) {
    
        return 0;
        
    }
    
    if (strlen(path) >= COMMC_PATH_MAX_LENGTH) {
    
        return 0;
        
    }
    
    /* Check each character */
    
    current = path;
    
    while (*current) {
    
        if (!is_valid_filename_char(*current) && !is_separator(*current)) {
        
            return 0;
            
        }
        
        current++;
        
    }
    
    return 1;
    
}

/*

         commc_path_exists()
	       ---
	       checks whether a path exists in the file system.

*/

int commc_path_exists(const char* path) {

#ifdef _WIN32

    DWORD attributes;
    
    if (!path) {
    
        return 0;
        
    }
    
    attributes = GetFileAttributesA(path);
    
    return (attributes != INVALID_FILE_ATTRIBUTES);
    
#else

    struct stat st;
    
    if (!path) {
    
        return 0;
        
    }
    
    return (stat(path, &st) == 0);
    
#endif

}

/*

         commc_path_get_type()
	       ---
	       analyzes a path string and determines its type.

*/

commc_path_type_t commc_path_get_type(const char* path) {

    if (!path || *path == '\0') {
    
        return COMMC_PATH_RELATIVE;
        
    }
    
#ifdef _WIN32

    /* Check for UNC path */
    
    if (path[0] == '\\' && path[1] == '\\') {
    
        return COMMC_PATH_UNC;
        
    }
    
    /* Check for drive letter */
    
    if (is_drive_letter(path[0]) && path[1] == ':') {
    
        if (path[2] == '\0') {
        
            return COMMC_PATH_DRIVE;  /* Drive-relative like C: */
            
        } else {
        
            return COMMC_PATH_ABSOLUTE;
            
        }
        
    }
    
#endif

    /* Check for current directory reference */
    
    if ((path[0] == '.' && (path[1] == '\0' || is_separator(path[1]))) ||
        (strncmp(path, "./", 2) == 0) || (strncmp(path, ".\\", 2) == 0)) {
        
        return COMMC_PATH_CURRENT;
        
    }
    
    /* Check for absolute path */
    
    if (is_separator(path[0])) {
    
        return COMMC_PATH_ABSOLUTE;
        
    }
    
    return COMMC_PATH_RELATIVE;
    
}

/* 
	==================================
           --- CONVERSION ---
	==================================
*/

/*

         commc_path_to_absolute()
	       ---
	       converts a relative path to an absolute path.

*/

commc_error_t commc_path_to_absolute(const char* relative_path,
                                     const char* base_path,
                                     char*       absolute_path,
                                     size_t      absolute_size) {

    char        working_buffer[COMMC_PATH_MAX_LENGTH];
    const char* base_to_use;
    commc_error_t result;
    
    if (!relative_path || !absolute_path || absolute_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* If already absolute, just copy */
    
    if (commc_path_is_absolute(relative_path)) {
    
        if (strlen(relative_path) >= absolute_size) {
        
            return COMMC_ERROR_BUFFER_TOO_SMALL;
            
        }
        
        strcpy(absolute_path, relative_path);
        return COMMC_SUCCESS;
        
    }
    
    /* Determine base path */
    
    if (base_path && *base_path) {
    
        base_to_use = base_path;
        
    } else {
    
        /* Use current working directory */
        
        if (!getcwd(working_buffer, sizeof(working_buffer))) {
        
            return COMMC_SYSTEM_ERROR;
            
        }
        
        base_to_use = working_buffer;
        
    }
    
    /* Join base and relative paths */
    
    result = commc_path_join(absolute_path, absolute_size, 
                            COMMC_PATH_FORMAT_NATIVE, 2,
                            base_to_use, relative_path);
                            
    if (result != COMMC_SUCCESS) {
    
        return result;
        
    }
    
    /* Normalize the result */
    
    strcpy(working_buffer, absolute_path);
    
    return commc_path_normalize(working_buffer, absolute_path, 
                               absolute_size, COMMC_PATH_FORMAT_NATIVE);
    
}

/*

         commc_path_to_relative()
	       ---
	       converts an absolute path to a relative path.

*/

commc_error_t commc_path_to_relative(const char* absolute_path,
                                     const char* base_path,
                                     char*       relative_path,
                                     size_t      relative_size) {

    /* This is a complex algorithm - simplified implementation */
    /* In a full implementation, this would properly calculate
       the relative path by comparing path components */
       
    if (!absolute_path || !base_path || !relative_path || relative_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* For now, just return the absolute path if conversion fails */
    
    if (strlen(absolute_path) >= relative_size) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    strcpy(relative_path, absolute_path);
    return COMMC_SUCCESS;
    
}

/*

         commc_path_convert_separators()
	       ---
	       converts path separators between formats.

*/

commc_error_t commc_path_convert_separators(const char*         input_path,
                                            char*               output_path,
                                            size_t              output_size,
                                            commc_path_format_t format) {

    char  target_separator;
    char* current;
    
    if (!input_path || !output_path || output_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (strlen(input_path) >= output_size) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    target_separator = get_format_separator(format);
    
    strcpy(output_path, input_path);
    current = output_path;
    
    while (*current) {
    
        if (is_separator(*current)) {
        
            *current = target_separator;
            
        }
        
        current++;
        
    }
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
           --- UTILITIES ---
	==================================
*/

/*

         commc_path_get_extension()
	       ---
	       extracts the file extension from a path.

*/

const char* commc_path_get_extension(const char* path) {

    const char* last_dot = NULL;
    const char* last_separator = NULL;
    const char* current;
    
    if (!path) {
    
        return NULL;
        
    }
    
    current = path;
    
    while (*current) {
    
        if (*current == '.') {
        
            last_dot = current;
            
        } else if (is_separator(*current)) {
        
            last_separator = current;
            last_dot = NULL;  /* Reset dot search after separator */
            
        }
        
        current++;
        
    }
    
    /* Make sure dot is in the filename portion */
    
    if (last_dot && (!last_separator || last_dot > last_separator)) {
    
        return last_dot;
        
    }
    
    return NULL;
    
}

/*

         commc_path_has_extension()
	       ---
	       checks whether a path contains a file extension.

*/

int commc_path_has_extension(const char* path,
                             const char* expected_extension) {

    const char* extension = commc_path_get_extension(path);
    
    if (!extension) {
    
        return 0;
        
    }
    
    if (expected_extension) {
    
        return (strcmp(extension, expected_extension) == 0);
        
    }
    
    return 1;
    
}

/*

         commc_path_get_separator()
	       ---
	       returns the appropriate path separator character.

*/

char commc_path_get_separator(commc_path_format_t format) {

    return get_format_separator(format);
    
}

/* 
	==================================
             --- EOF ---
	==================================
*/
