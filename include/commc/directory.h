/*

   ===================================
   C O M M C   -   D I R E C T O R Y
   DIRECTORY TRAVERSAL HEADER
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_DIRECTORY_H
#define COMMC_DIRECTORY_H

#include "error.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
         --- DATA TYPES ---
	==================================
*/

/*
   directory entry information structure
   containing path, name, type, and attributes
   for comprehensive file system navigation
   and analysis operations
*/

typedef struct {

    char* path;          /* full path to entry */
    char* name;          /* entry name only */
    char* extension;     /* file extension (NULL for directories) */

    int is_directory;    /* 1 if directory, 0 if file */
    int is_hidden;       /* 1 if hidden entry */
    int is_readable;     /* 1 if readable */
    int is_writable;     /* 1 if writable */

    long size;           /* file size in bytes (-1 for directories) */
    long modified_time;  /* last modification time (seconds since epoch) */

} commc_directory_entry_t;

/*
   directory traversal context for configuration
   and state management during recursive walking
   operations with filtering and callback support
*/

typedef struct {

    int recursive;                /* enable recursive traversal */
    int include_hidden;           /* include hidden files/directories */
    int include_directories;      /* include directories in results */
    int include_files;            /* include files in results */

    const char* pattern;           /* filename pattern filter (wildcards) */
    const char* extension_filter;  /* file extension filter */

    int max_depth;                  /* maximum recursion depth (-1 = unlimited) */
    int follow_symlinks;            /* follow symbolic links */

    /* callback for custom filtering */

    int (*filter_callback)(const commc_directory_entry_t* entry, void* user_data);
    void* user_data;            /* user data for callback */

    /* error context */

    commc_error_context_t error;

} commc_directory_context_t;

/*
   directory listing result containing array
   of entries and metadata for traversal
   results management and iteration
*/

typedef struct {

    commc_directory_entry_t* entries;  /* array of directory entries */
    int count;                         /* number of entries */
    int capacity;                      /* allocated capacity */

    int total_files;                   /* total file count */
    int total_directories;             /* total directory count */
    long total_size;                   /* total size of all files */

} commc_directory_listing_t;

/* 
	==================================
     --- CALLBACK TYPES ---
	==================================
*/

/*
   callback function for directory traversal
   called for each entry during walking operation
   return 0 to continue, non-zero to stop
*/

typedef int (*commc_directory_walk_callback_t)(const commc_directory_entry_t* entry,
                                               int depth,
                                               void* user_data);

/*
   callback function for directory change monitoring
   called when file system changes detected
   providing change type and entry information
*/

typedef void (*commc_directory_change_callback_t)(const char* path,
                                                  int change_type,
                                                  void* user_data);

/* 
	==================================
       --- CONSTANTS ---
	==================================
*/

/* directory change types for monitoring */

#define COMMC_DIR_CHANGE_CREATED     1
#define COMMC_DIR_CHANGE_MODIFIED    2
#define COMMC_DIR_CHANGE_DELETED     3
#define COMMC_DIR_CHANGE_RENAMED     4

/* wildcard pattern matching constants */

#define COMMC_DIR_PATTERN_CASE_SENSITIVE   0x01
#define COMMC_DIR_PATTERN_CASE_INSENSITIVE 0x02

/* 
	==================================
         --- CORE API ---
	==================================
*/

/*
   create directory traversal context with
   default configuration for file system
   navigation and filtering operations
*/

commc_directory_context_t* commc_directory_context_create(void);

/*
   destroy directory context and free
   associated memory including callbacks
   and filter configuration data
*/

void commc_directory_context_destroy(commc_directory_context_t* context);

/*
   set pattern filter for filename matching
   using wildcards (* and ?) with optional
   case sensitivity configuration
*/

int commc_directory_set_pattern(commc_directory_context_t* context,
                               const char* pattern,
                               int flags);

/*
   set file extension filter to include
   only files with specified extension
   during traversal operations
*/

int commc_directory_set_extension_filter(commc_directory_context_t* context,
                                        const char* extension);

/*
   set custom filter callback for advanced
   entry filtering during directory
   traversal and enumeration operations
*/

int commc_directory_set_filter_callback(commc_directory_context_t* context,
                                       int (*callback)(const commc_directory_entry_t*, void*),
                                       void* user_data);

/* 
	==================================
       --- LISTING API ---
	==================================
*/

/*
   create directory listing result structure
   with initial capacity for entry storage
   and traversal metadata management
*/

commc_directory_listing_t* commc_directory_listing_create(void);

/*
   destroy directory listing and free all
   associated memory including entry data
   and path strings
*/

void commc_directory_listing_destroy(commc_directory_listing_t* listing);

/*
   list directory contents with filtering
   and configuration options returning
   populated listing structure
*/

commc_directory_listing_t* commc_directory_list(commc_directory_context_t* context,
                                               const char* path);

/*
   recursively list directory contents
   with depth control and filtering
   for comprehensive directory analysis
*/

commc_directory_listing_t* commc_directory_list_recursive(commc_directory_context_t* context,
                                                         const char* path,
                                                         int max_depth);

/*
   add entry to directory listing with
   automatic capacity management and
   metadata updates
*/

int commc_directory_listing_add_entry(commc_directory_listing_t* listing,
                                     const commc_directory_entry_t* entry);

/*
   get entry from listing by index
   with bounds checking for safe
   iteration and access
*/

const commc_directory_entry_t* commc_directory_listing_get_entry(const commc_directory_listing_t* listing,
                                                                int index);

/*
   sort directory listing by specified
   criteria including name, size, and
   modification time with direction control
*/

int commc_directory_listing_sort(commc_directory_listing_t* listing,
                               int sort_type,
                               int ascending);

/* 
	==================================
      --- TRAVERSAL API ---
	==================================
*/

/*
   walk directory tree with callback
   for each entry providing depth
   information and traversal control
*/

int commc_directory_walk(commc_directory_context_t* context,
                        const char* path,
                        commc_directory_walk_callback_t callback,
                        void* user_data);

/*
   walk directory tree recursively with
   maximum depth control and callback
   for comprehensive tree traversal
*/

int commc_directory_walk_recursive(commc_directory_context_t* context,
                                  const char* path,
                                  int max_depth,
                                  commc_directory_walk_callback_t callback,
                                  void* user_data);

/*
   find files matching pattern in directory
   tree with recursive search and filtering
   capabilities returning match listing
*/

commc_directory_listing_t* commc_directory_find(commc_directory_context_t* context,
                                               const char* root_path,
                                               const char* pattern);

/*
   find files by extension in directory
   tree with recursive search returning
   filtered listing of matching files
*/

commc_directory_listing_t* commc_directory_find_by_extension(commc_directory_context_t* context,
                                                           const char* root_path,
                                                           const char* extension);

/*
   find files by size criteria with
   minimum and maximum size filtering
   during recursive directory traversal
*/

commc_directory_listing_t* commc_directory_find_by_size(commc_directory_context_t* context,
                                                       const char* root_path,
                                                       long min_size,
                                                       long max_size);

/* 
	==================================
       --- UTILITY API ---
	==================================
*/

/*
   check if path exists and is accessible
   with permission and type validation
   for file system operations
*/

int commc_directory_exists(const char* path);

/*
   check if path is directory with
   accessibility validation for
   directory-specific operations
*/

int commc_directory_is_directory(const char* path);

/*
   check if path is regular file with
   accessibility validation for
   file-specific operations
*/

int commc_directory_is_file(const char* path);

/*
   get directory entry information
   for specified path including
   attributes and metadata
*/

commc_directory_entry_t* commc_directory_get_entry_info(const char* path);

/*
   destroy directory entry and free
   associated memory including path
   and name strings
*/

void commc_directory_entry_destroy(commc_directory_entry_t* entry);

/*
   create directory with parents if needed
   similar to mkdir -p functionality
   with cross-platform support
*/

int commc_directory_create(const char* path);

/*
   remove empty directory with
   validation and error handling
   for safe directory deletion
*/

int commc_directory_remove(const char* path);

/*
   remove directory recursively with
   all contents including subdirectories
   and files with safety checks
*/

int commc_directory_remove_recursive(const char* path);

/* 
	==================================
       --- PATTERN API ---
	==================================
*/

/*
   match filename against pattern using
   wildcards with case sensitivity
   options for flexible filtering
*/

int commc_directory_pattern_match(const char* filename,
                                 const char* pattern,
                                 int flags);

/*
   match multiple patterns against filename
   with OR logic returning match on any
   pattern success
*/

int commc_directory_pattern_match_list(const char* filename,
                                      const char** patterns,
                                      int pattern_count,
                                      int flags);

/*
   compile pattern for efficient repeated
   matching operations with optimization
   for high-frequency pattern matching
*/

void* commc_directory_pattern_compile(const char* pattern, int flags);

/*
   match filename against compiled pattern
   for efficient repeated matching without
   pattern recompilation overhead
*/

int commc_directory_pattern_match_compiled(const char* filename,
                                          void* compiled_pattern);

/*
   destroy compiled pattern and free
   associated memory for pattern
   matching optimization cleanup
*/

void commc_directory_pattern_destroy(void* compiled_pattern);

/* 
	==================================
         --- SORT API ---
	==================================
*/

/* sort types for directory listings */
#define COMMC_DIR_SORT_NAME      1  /* sort by filename */
#define COMMC_DIR_SORT_SIZE      2  /* sort by file size */
#define COMMC_DIR_SORT_MODIFIED  3  /* sort by modification time */
#define COMMC_DIR_SORT_TYPE      4  /* sort by file type */
#define COMMC_DIR_SORT_EXTENSION 5  /* sort by file extension */

/* sort directions */
#define COMMC_DIR_SORT_ASCENDING  1
#define COMMC_DIR_SORT_DESCENDING 0

/*
   sort directory entries using custom
   comparison function for specialized
   sorting requirements
*/

int commc_directory_sort_custom(commc_directory_listing_t* listing,
                               int (*compare_func)(const commc_directory_entry_t*,
                                                 const commc_directory_entry_t*));

/* 
	==================================
      --- STATISTICS API ---
	==================================
*/

/*
   directory statistics structure for
   comprehensive directory analysis
   and reporting capabilities
*/

typedef struct {

    int total_entries;      /* total entries processed */
    int total_files;        /* total files found */
    int total_directories;  /* total directories found */
    
    long total_size;        /* total size of all files */
    long largest_file;      /* size of largest file */
    long smallest_file;     /* size of smallest file */
    long average_file_size; /* average file size */

    int max_depth;          /* maximum depth reached */
    int total_depth;        /* total depth traversed */

    long start_time;        /* traversal start time */
    long end_time;          /* traversal end time */
    long duration;          /* traversal duration */

} commc_directory_stats_t;

/*
   gather directory statistics during
   traversal with comprehensive metrics
   for analysis and reporting
*/

commc_directory_stats_t* commc_directory_gather_stats(commc_directory_context_t* context,
                                                     const char* path);

/*
   print directory statistics in
   human-readable format with
   detailed breakdown and metrics
*/

void commc_directory_print_stats(const commc_directory_stats_t* stats);

/*
   destroy directory statistics structure
   and free associated memory for
   cleanup after analysis operations
*/

void commc_directory_stats_destroy(commc_directory_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_DIRECTORY_H */

/* 
	==================================
           --- EOF ---
	==================================
*/