/*

   ===================================
   C O M M C   -   D I R E C T O R Y
   DIRECTORY TRAVERSAL UTILITIES
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#include "commc/directory.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fnmatch.h>
#endif

/* 
	==================================
         --- CONSTANTS ---
	==================================
*/

#define DIRECTORY_INITIAL_CAPACITY  64
#define DIRECTORY_MAX_PATH_LENGTH   4096

/* 
	==================================
         --- HELPERS ---
	==================================
*/

/*

         get_file_stats()
	       ---
	       retrieves file system information for
	       specified path including size, modification
	       time, and access permissions using
	       platform-specific system calls.

*/

static int get_file_stats(const char* path, commc_directory_entry_t* entry) {

#ifdef _WIN32
    struct _stat file_stat;
    
    if (_stat(path, &file_stat) != 0) {
        return -1;
    }

    entry->size = file_stat.st_size;
    entry->modified_time = file_stat.st_mtime;
    entry->is_directory = (file_stat.st_mode & S_IFDIR) != 0;
    
    /* Windows permission checking */
    entry->is_readable = _access(path, 4) == 0;  /* R_OK */
    entry->is_writable = _access(path, 2) == 0;  /* W_OK */

#else
    struct stat file_stat;
    
    if (stat(path, &file_stat) != 0) {
        return -1;
    }

    entry->size = file_stat.st_size;
    entry->modified_time = file_stat.st_mtime;
    entry->is_directory = S_ISDIR(file_stat.st_mode);
    
    /* Unix permission checking */
    entry->is_readable = access(path, R_OK) == 0;
    entry->is_writable = access(path, W_OK) == 0;

#endif

    return 0;
}

/*

         is_hidden_entry()
	       ---
	       determines if directory entry is hidden
	       based on platform-specific conventions
	       like leading dot on Unix or file
	       attributes on Windows systems.

*/

static int is_hidden_entry(const char* name, const char* path) {

#ifdef _WIN32
    DWORD attributes;
    (void)name;  /* suppress unused parameter warning */
    
    attributes = GetFileAttributesA(path);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    }
    return 0;
#else
    /* Unix-style hidden files start with dot */
    (void)path;  /* suppress unused parameter warning */
    return name[0] == '.' && strcmp(name, ".") != 0 && strcmp(name, "..") != 0;
#endif
}

/*

         extract_file_extension()
	       ---
	       extracts file extension from filename
	       returning pointer to extension string
	       or NULL if no extension present in
	       the filename.

*/

static char* extract_file_extension(const char* filename) {

    const char* dot;
    const char* last_separator;

    if (!filename || strlen(filename) == 0) {
        return NULL;
    }

    /* find last directory separator */
    last_separator = strrchr(filename, '/');
    
    if (!last_separator) {
        last_separator = strrchr(filename, '\\');
    }

    /* find last dot after last separator */
    dot = strrchr(last_separator ? last_separator : filename, '.');

    if (!dot || dot == filename || dot == last_separator + 1) {
        return NULL;
    }

    return strdup(dot + 1);  /* return allocated extension without dot */
}

/*

         wildcard_match()
	       ---
	       performs wildcard pattern matching
	       supporting * and ? metacharacters
	       with optional case sensitivity for
	       flexible filename filtering.

*/

static int wildcard_match(const char* pattern, const char* string, int case_sensitive) {

    const char* p = pattern;
    const char* s = string;
    const char* star = NULL;
    const char* ss = s;

    while (*s) {

        if (*p == '?') {
            /* single character match */
            p++;
            s++;
        } else if (*p == '*') {
            /* multi-character match */
            star = p++;
            ss = s;
        } else {
            /* literal character match */
            int p_char = case_sensitive ? *p : tolower(*p);
            int s_char = case_sensitive ? *s : tolower(*s);

            if (p_char == s_char) {
                p++;
                s++;
            } else if (star) {
                /* backtrack to star */
                p = star + 1;
                s = ++ss;
            } else {
                return 0;
            }
        }
    }

    /* consume remaining stars */
    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}

/* 
	==================================
         --- CONTEXT API ---
	==================================
*/

/*

         commc_directory_context_create()
	       ---
	       creates directory traversal context with
	       default configuration for recursive
	       walking, pattern matching, and filtering
	       operations with error handling.

*/

commc_directory_context_t* commc_directory_context_create(void) {

    commc_directory_context_t* context;

    context = (commc_directory_context_t*)malloc(sizeof(commc_directory_context_t));

    if (!context) {
        return NULL;
    }

    /* set default configuration */
    context->recursive = 1;
    context->include_hidden = 0;
    context->include_directories = 1;
    context->include_files = 1;

    context->pattern = NULL;
    context->extension_filter = NULL;

    context->max_depth = -1;  /* unlimited */
    context->follow_symlinks = 0;

    context->filter_callback = NULL;
    context->user_data = NULL;

    memset(&context->error, 0, sizeof(commc_error_context_t));

    return context;
}

/*

         commc_directory_context_destroy()
	       ---
	       releases all memory associated with
	       directory context including pattern
	       strings and callback data for complete
	       cleanup of traversal resources.

*/

void commc_directory_context_destroy(commc_directory_context_t* context) {

    if (!context) {
        return;
    }

    if (context->pattern) {
        free((void*)context->pattern);
    }

    if (context->extension_filter) {
        free((void*)context->extension_filter);
    }

    free(context);
}

/*

         commc_directory_set_pattern()
	       ---
	       sets filename pattern for filtering with
	       wildcard support and case sensitivity
	       configuration for flexible file
	       matching during traversal.

*/

int commc_directory_set_pattern(commc_directory_context_t* context,
                               const char* pattern,
                               int flags) {

    if (!context || !pattern) {
        return -1;
    }

    (void)flags;  /* suppress unused parameter warning for now */

    /* free existing pattern */
    if (context->pattern) {
        free((void*)context->pattern);
    }

    context->pattern = strdup(pattern);

    return context->pattern ? 0 : -1;
}

/*

         commc_directory_set_extension_filter()
	       ---
	       sets file extension filter to include
	       only files with specified extension
	       during directory traversal and
	       enumeration operations.

*/

int commc_directory_set_extension_filter(commc_directory_context_t* context,
                                        const char* extension) {

    if (!context || !extension) {
        return -1;
    }

    /* free existing filter */
    if (context->extension_filter) {
        free((void*)context->extension_filter);
    }

    context->extension_filter = strdup(extension);

    return context->extension_filter ? 0 : -1;
}

/*

         commc_directory_set_filter_callback()
	       ---
	       sets custom filter callback function
	       for advanced entry filtering during
	       directory traversal with user data
	       support for flexible filtering logic.

*/

int commc_directory_set_filter_callback(commc_directory_context_t* context,
                                       int (*callback)(const commc_directory_entry_t*, void*),
                                       void* user_data) {

    if (!context) {
        return -1;
    }

    context->filter_callback = callback;
    context->user_data = user_data;

    return 0;
}

/* 
	==================================
        --- LISTING API ---
	==================================
*/

/*

         commc_directory_listing_create()
	       ---
	       creates directory listing result with
	       initial capacity for entry storage
	       and metadata tracking for traversal
	       results management.

*/

commc_directory_listing_t* commc_directory_listing_create(void) {

    commc_directory_listing_t* listing;

    listing = (commc_directory_listing_t*)malloc(sizeof(commc_directory_listing_t));

    if (!listing) {
        return NULL;
    }

    listing->entries = (commc_directory_entry_t*)malloc(DIRECTORY_INITIAL_CAPACITY * sizeof(commc_directory_entry_t));

    if (!listing->entries) {
        free(listing);
        return NULL;
    }

    listing->count = 0;
    listing->capacity = DIRECTORY_INITIAL_CAPACITY;

    listing->total_files = 0;
    listing->total_directories = 0;
    listing->total_size = 0;

    return listing;
}

/*

         commc_directory_listing_destroy()
	       ---
	       releases all memory associated with
	       directory listing including entry
	       data and path strings for complete
	       cleanup of listing resources.

*/

void commc_directory_listing_destroy(commc_directory_listing_t* listing) {

    int i;

    if (!listing) {
        return;
    }

    if (listing->entries) {

        for (i = 0; i < listing->count; i++) {

            if (listing->entries[i].path) {
                free(listing->entries[i].path);
            }

            if (listing->entries[i].name) {
                free(listing->entries[i].name);
            }

            if (listing->entries[i].extension) {
                free(listing->entries[i].extension);
            }
        }

        free(listing->entries);
    }

    free(listing);
}

/*

         should_include_entry()
	       ---
	       determines if directory entry should be
	       included in results based on context
	       configuration and filtering criteria
	       including pattern and callback filtering.

*/

static int should_include_entry(const commc_directory_context_t* context, const commc_directory_entry_t* entry) {

    /* check hidden files */
    if (!context->include_hidden && entry->is_hidden) {
        return 0;
    }

    /* check file vs directory inclusion */
    if (entry->is_directory && !context->include_directories) {
        return 0;
    }

    if (!entry->is_directory && !context->include_files) {
        return 0;
    }

    /* check pattern filter */
    if (context->pattern) {

        if (!wildcard_match(context->pattern, entry->name, 1)) {
            return 0;
        }
    }

    /* check extension filter */
    if (context->extension_filter && !entry->is_directory) {

        if (!entry->extension ||
            strcmp(entry->extension, context->extension_filter) != 0) {
            return 0;
        }
    }

    /* check custom filter callback */
    if (context->filter_callback) {

        if (!context->filter_callback(entry, context->user_data)) {
            return 0;
        }
    }

    return 1;
}

/*

         commc_directory_listing_add_entry()
	       ---
	       adds entry to directory listing with
	       automatic capacity management and
	       metadata updates for efficient
	       listing growth.

*/

int commc_directory_listing_add_entry(commc_directory_listing_t* listing,
                                     const commc_directory_entry_t* entry) {

    commc_directory_entry_t* new_entries;

    if (!listing || !entry) {
        return -1;
    }

    /* expand capacity if needed */
    if (listing->count >= listing->capacity) {

        int new_capacity = listing->capacity * 2;

        new_entries = (commc_directory_entry_t*)realloc(listing->entries,
                                                       new_capacity * sizeof(commc_directory_entry_t));

        if (!new_entries) {
            return -1;
        }

        listing->entries = new_entries;
        listing->capacity = new_capacity;
    }

    /* copy entry */
    listing->entries[listing->count] = *entry;

    /* duplicate strings */
    listing->entries[listing->count].path = entry->path ? strdup(entry->path) : NULL;
    listing->entries[listing->count].name = entry->name ? strdup(entry->name) : NULL;
    listing->entries[listing->count].extension = entry->extension ? strdup(entry->extension) : NULL;

    /* update metadata */
    if (entry->is_directory) {
        listing->total_directories++;
    } else {
        listing->total_files++;
        if (entry->size > 0) {
            listing->total_size += entry->size;
        }
    }

    listing->count++;

    return 0;
}

/*

         commc_directory_list()
	       ---
	       lists directory contents with filtering
	       and configuration options from context
	       returning populated listing structure
	       for iteration and analysis.

*/

commc_directory_listing_t* commc_directory_list(commc_directory_context_t* context,
                                               const char* path) {

    commc_directory_listing_t* listing;
    commc_directory_entry_t entry;

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle;
    char search_path[DIRECTORY_MAX_PATH_LENGTH];
    
    if (!context || !path) {
        return NULL;
    }

    listing = commc_directory_listing_create();
    
    if (!listing) {
        return NULL;
    }

    /* create search pattern */
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    find_handle = FindFirstFileA(search_path, &find_data);

    if (find_handle == INVALID_HANDLE_VALUE) {
        commc_directory_listing_destroy(listing);
        return NULL;
    }

    do {
        /* skip . and .. */
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        /* build full path */
        snprintf(search_path, sizeof(search_path), "%s\\%s", path, find_data.cFileName);

        /* populate entry */
        memset(&entry, 0, sizeof(entry));
        
        entry.path = search_path;
        entry.name = find_data.cFileName;
        entry.extension = extract_file_extension(find_data.cFileName);
        entry.is_hidden = is_hidden_entry(find_data.cFileName, search_path);

        if (get_file_stats(search_path, &entry) == 0) {

            if (should_include_entry(context, &entry)) {
                commc_directory_listing_add_entry(listing, &entry);
            }
        }

    } while (FindNextFileA(find_handle, &find_data));

    FindClose(find_handle);

#else
    DIR* dir;
    struct dirent* dir_entry;
    char full_path[DIRECTORY_MAX_PATH_LENGTH];

    if (!context || !path) {
        return NULL;
    }

    listing = commc_directory_listing_create();
    
    if (!listing) {
        return NULL;
    }

    dir = opendir(path);

    if (!dir) {
        commc_directory_listing_destroy(listing);
        return NULL;
    }

    while ((dir_entry = readdir(dir)) != NULL) {

        /* skip . and .. */
        if (strcmp(dir_entry->d_name, ".") == 0 || 
            strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        /* build full path */
        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir_entry->d_name);

        /* populate entry */
        memset(&entry, 0, sizeof(entry));
        
        entry.path = full_path;
        entry.name = dir_entry->d_name;
        entry.extension = extract_file_extension(dir_entry->d_name);
        entry.is_hidden = is_hidden_entry(dir_entry->d_name, full_path);

        if (get_file_stats(full_path, &entry) == 0) {

            if (should_include_entry(context, &entry)) {
                commc_directory_listing_add_entry(listing, &entry);
            }
        }
    }

    closedir(dir);
#endif

    return listing;
}

/*

         commc_directory_list_recursive()
	       ---
	       recursively lists directory contents
	       with depth control and filtering for
	       comprehensive directory tree analysis
	       and enumeration.

*/

commc_directory_listing_t* commc_directory_list_recursive(commc_directory_context_t* context,
                                                         const char* path,
                                                         int max_depth) {

    commc_directory_listing_t* result;
    commc_directory_listing_t* current_level;
    int i;
    int original_max_depth;

    if (!context || !path || max_depth == 0) {
        return commc_directory_list(context, path);
    }

    /* save and set max depth */
    original_max_depth = context->max_depth;
    context->max_depth = max_depth;

    result = commc_directory_listing_create();

    if (!result) {
        return NULL;
    }

    /* get current level */
    current_level = commc_directory_list(context, path);

    if (!current_level) {
        commc_directory_listing_destroy(result);
        context->max_depth = original_max_depth;
        return NULL;
    }

    /* add all entries from current level */
    for (i = 0; i < current_level->count; i++) {
        commc_directory_listing_add_entry(result, &current_level->entries[i]);
    }

    /* recurse into subdirectories */
    if (max_depth > 1) {

        for (i = 0; i < current_level->count; i++) {

            if (current_level->entries[i].is_directory) {

                commc_directory_listing_t* sub_listing;
                int j;

                sub_listing = commc_directory_list_recursive(context,
                                                           current_level->entries[i].path,
                                                           max_depth - 1);

                if (sub_listing) {

                    for (j = 0; j < sub_listing->count; j++) {
                        commc_directory_listing_add_entry(result, &sub_listing->entries[j]);
                    }

                    commc_directory_listing_destroy(sub_listing);
                }
            }
        }
    }

    commc_directory_listing_destroy(current_level);
    context->max_depth = original_max_depth;

    return result;
}

/* 
	==================================
        --- UTILITY API ---
	==================================
*/

/*

         commc_directory_exists()
	       ---
	       checks if path exists and is accessible
	       using platform-specific system calls
	       for file system validation and
	       path verification.

*/

int commc_directory_exists(const char* path) {

#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES;
#else
    struct stat path_stat;
    return stat(path, &path_stat) == 0;
#endif
}

/*

         commc_directory_is_directory()
	       ---
	       checks if path is directory with
	       accessibility validation for
	       directory-specific operations and
	       traversal capabilities.

*/

int commc_directory_is_directory(const char* path) {

#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES) && 
           (attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat path_stat;
    return (stat(path, &path_stat) == 0) && S_ISDIR(path_stat.st_mode);
#endif
}

/*

         commc_directory_is_file()
	       ---
	       checks if path is regular file with
	       accessibility validation for
	       file-specific operations and
	       processing workflows.

*/

int commc_directory_is_file(const char* path) {

#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES) && 
           !(attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat path_stat;
    return (stat(path, &path_stat) == 0) && S_ISREG(path_stat.st_mode);
#endif
}

/*

         commc_directory_get_entry_info()
	       ---
	       retrieves comprehensive directory entry
	       information for specified path including
	       attributes, permissions, and metadata
	       for analysis and processing.

*/

commc_directory_entry_t* commc_directory_get_entry_info(const char* path) {

    commc_directory_entry_t* entry;
    const char* filename;
    const char* last_separator;

    if (!path || !commc_directory_exists(path)) {
        return NULL;
    }

    entry = (commc_directory_entry_t*)malloc(sizeof(commc_directory_entry_t));

    if (!entry) {
        return NULL;
    }

    memset(entry, 0, sizeof(commc_directory_entry_t));

    /* set path */
    entry->path = strdup(path);

    /* extract filename */
    last_separator = strrchr(path, '/');
    
    if (!last_separator) {
        last_separator = strrchr(path, '\\');
    }

    filename = last_separator ? last_separator + 1 : path;
    entry->name = strdup(filename);

    /* extract extension */
    entry->extension = extract_file_extension(filename);

    /* get file stats */
    if (get_file_stats(path, entry) != 0) {
        commc_directory_entry_destroy(entry);
        return NULL;
    }

    /* check if hidden */
    entry->is_hidden = is_hidden_entry(filename, path);

    return entry;
}

/*

         commc_directory_entry_destroy()
	       ---
	       releases all memory associated with
	       directory entry including path and
	       name strings for complete cleanup
	       of entry resources.

*/

void commc_directory_entry_destroy(commc_directory_entry_t* entry) {

    if (!entry) {
        return;
    }

    if (entry->path) {
        free(entry->path);
    }

    if (entry->name) {
        free(entry->name);
    }

    if (entry->extension) {
        free(entry->extension);
    }

    free(entry);
}

/*

         commc_directory_pattern_match()
	       ---
	       performs wildcard pattern matching
	       against filename with case sensitivity
	       options for flexible file filtering
	       and selection operations.

*/

int commc_directory_pattern_match(const char* filename,
                                 const char* pattern,
                                 int flags) {

    int case_sensitive;

    if (!filename || !pattern) {
        return 0;
    }

    case_sensitive = (flags & COMMC_DIR_PATTERN_CASE_SENSITIVE) != 0;

    return wildcard_match(pattern, filename, case_sensitive);
}

/* 
	==================================
           --- EOF ---
	==================================
*/