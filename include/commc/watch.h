/*

   ===================================
   C O M M C   -   W A T C H
   FILE AND DIRECTORY MONITORING
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_WATCH_H
#define COMMC_WATCH_H

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
   file system change event structure
   containing information about detected
   changes including path, type, and
   additional event metadata
*/

typedef struct {

    char* path;              /* path to changed file or directory */
    char* old_path;          /* previous path for rename operations */

    int event_type;          /* type of change event */
    int is_directory;        /* 1 if change affects directory */

    long timestamp;          /* event timestamp (seconds since epoch) */
    long file_size;          /* file size after change (-1 if unknown) */

    void* user_data;         /* custom user data for callback */

} commc_watch_event_t;

/*
   file monitoring context for watch
   configuration and state management
   during file system monitoring operations
*/

typedef struct {

    char* watch_path;        /* root path being monitored */
    int recursive;           /* monitor subdirectories recursively */
    int watch_files;         /* monitor file changes */
    int watch_directories;   /* monitor directory changes */

    int filter_mask;         /* event type filter mask */
    long debounce_time;      /* minimum time between events (ms) */

    /* platform-specific monitoring handle */
    void* platform_handle;

    /* callback for change events */
    void (*event_callback)(const commc_watch_event_t* event, void* user_data);
    void* user_data;         /* user data for callback */

    /* error context */
    commc_error_context_t error;

    /* internal state */
    int is_monitoring;       /* monitoring active flag */
    int stop_requested;      /* stop monitoring flag */

} commc_watch_context_t;

/*
   batch monitoring context for watching
   multiple paths with shared configuration
   and event handling for efficient
   multi-path file system monitoring
*/

typedef struct {

    commc_watch_context_t** watches;  /* array of watch contexts */
    int watch_count;                  /* number of active watches */
    int capacity;                     /* allocated watch capacity */

    /* shared configuration */
    int default_recursive;
    int default_filter_mask;
    long default_debounce_time;

    /* batch event callback */
    void (*batch_callback)(const commc_watch_event_t* events,
                          int event_count,
                          void* user_data);
    void* user_data;

    /* thread management */
    void* monitor_thread;
    int thread_running;

} commc_watch_batch_t;

/* 
	==================================
       --- CONSTANTS ---
	==================================
*/

/* file system event types */
#define COMMC_WATCH_FILE_CREATED     0x0001
#define COMMC_WATCH_FILE_MODIFIED    0x0002
#define COMMC_WATCH_FILE_DELETED     0x0004
#define COMMC_WATCH_FILE_MOVED       0x0008
#define COMMC_WATCH_FILE_RENAMED     0x0010
#define COMMC_WATCH_FILE_ATTRIBUTES  0x0020

#define COMMC_WATCH_DIR_CREATED      0x0040
#define COMMC_WATCH_DIR_MODIFIED     0x0080
#define COMMC_WATCH_DIR_DELETED      0x0100
#define COMMC_WATCH_DIR_MOVED        0x0200
#define COMMC_WATCH_DIR_RENAMED      0x0400

/* combined event masks for convenience */
#define COMMC_WATCH_ALL_FILES        (COMMC_WATCH_FILE_CREATED | COMMC_WATCH_FILE_MODIFIED | \
                                      COMMC_WATCH_FILE_DELETED | COMMC_WATCH_FILE_MOVED | \
                                      COMMC_WATCH_FILE_RENAMED | COMMC_WATCH_FILE_ATTRIBUTES)

#define COMMC_WATCH_ALL_DIRECTORIES  (COMMC_WATCH_DIR_CREATED | COMMC_WATCH_DIR_MODIFIED | \
                                      COMMC_WATCH_DIR_DELETED | COMMC_WATCH_DIR_MOVED | \
                                      COMMC_WATCH_DIR_RENAMED)

#define COMMC_WATCH_ALL_CHANGES      (COMMC_WATCH_ALL_FILES | COMMC_WATCH_ALL_DIRECTORIES)

/* monitoring flags */
#define COMMC_WATCH_RECURSIVE        0x1000
#define COMMC_WATCH_FOLLOW_SYMLINKS  0x2000
#define COMMC_WATCH_CASE_SENSITIVE   0x4000

/* 
	==================================
         --- CORE API ---
	==================================
*/

/*
   create file monitoring context with
   default configuration for watching
   file system changes and events
*/

commc_watch_context_t* commc_watch_create(const char* path);

/*
   destroy watch context and stop
   monitoring operations including
   cleanup of platform resources
*/

void commc_watch_destroy(commc_watch_context_t* watch);

/*
   configure monitoring options including
   recursion, event filtering, and
   debounce timing for optimal performance
*/

int commc_watch_set_options(commc_watch_context_t* watch,
                           int recursive,
                           int event_mask,
                           long debounce_ms);

/*
   set callback function for file system
   events with user data support for
   custom event handling and processing
*/

int commc_watch_set_callback(commc_watch_context_t* watch,
                            void (*callback)(const commc_watch_event_t*, void*),
                            void* user_data);

/*
   start file system monitoring with
   specified configuration and begin
   detecting and reporting changes
*/

int commc_watch_start(commc_watch_context_t* watch);

/*
   stop file system monitoring and
   cease change detection while
   preserving context configuration
*/

int commc_watch_stop(commc_watch_context_t* watch);

/*
   check if monitoring is currently
   active for specified watch context
   returning status information
*/

int commc_watch_is_active(const commc_watch_context_t* watch);

/* 
	==================================
       --- POLLING API ---
	==================================
*/

/*
   poll for file system changes without
   callback using synchronous detection
   for integration with custom event loops
*/

int commc_watch_poll(commc_watch_context_t* watch,
                    commc_watch_event_t** events,
                    int* event_count,
                    long timeout_ms);

/*
   wait for single file system event
   with timeout support for blocking
   event detection and processing
*/

commc_watch_event_t* commc_watch_wait_event(commc_watch_context_t* watch,
                                           long timeout_ms);

/*
   process pending events manually
   for custom event loop integration
   and batch event handling
*/

int commc_watch_process_events(commc_watch_context_t* watch);

/* 
	==================================
       --- BATCH API ---
	==================================
*/

/*
   create batch monitoring context for
   watching multiple paths with shared
   configuration and event management
*/

commc_watch_batch_t* commc_watch_batch_create(void);

/*
   destroy batch context and stop all
   monitoring operations including
   cleanup of watch contexts and resources
*/

void commc_watch_batch_destroy(commc_watch_batch_t* batch);

/*
   add path to batch monitoring with
   individual configuration options
   for flexible multi-path watching
*/

int commc_watch_batch_add(commc_watch_batch_t* batch,
                         const char* path,
                         int recursive,
                         int event_mask);

/*
   remove path from batch monitoring
   and stop watching specified location
   while maintaining other watches
*/

int commc_watch_batch_remove(commc_watch_batch_t* batch,
                            const char* path);

/*
   start all batch monitoring operations
   with shared event handling and
   efficient resource utilization
*/

int commc_watch_batch_start(commc_watch_batch_t* batch);

/*
   stop all batch monitoring operations
   while preserving batch configuration
   for potential restart
*/

int commc_watch_batch_stop(commc_watch_batch_t* batch);

/*
   set batch event callback for handling
   multiple events efficiently with
   batch processing capabilities
*/

int commc_watch_batch_set_callback(commc_watch_batch_t* batch,
                                  void (*callback)(const commc_watch_event_t*,
                                                  int, void*),
                                  void* user_data);

/* 
	==================================
        --- UTILITY API ---
	==================================
*/

/*
   get event type name as human-readable
   string for logging and debugging
   purposes during event processing
*/

const char* commc_watch_event_type_name(int event_type);

/*
   check if path supports file system
   monitoring on current platform
   for capability validation
*/

int commc_watch_path_supported(const char* path);

/*
   get platform-specific monitoring
   capabilities and limitations
   for optimal configuration
*/

int commc_watch_get_capabilities(void);

/*
   create event structure for manual
   event generation during testing
   and simulation scenarios
*/

commc_watch_event_t* commc_watch_event_create(const char* path,
                                             int event_type,
                                             int is_directory);

/*
   destroy watch event and free
   associated memory including
   path strings and metadata
*/

void commc_watch_event_destroy(commc_watch_event_t* event);

/*
   copy watch event for storage
   and delayed processing with
   complete data duplication
*/

commc_watch_event_t* commc_watch_event_copy(const commc_watch_event_t* event);

/* 
	==================================
        --- FILTER API ---
	==================================
*/

/*
   file extension filter structure
   for filtering events by file type
   during monitoring operations
*/

typedef struct {

    char** extensions;      /* array of file extensions */
    int count;             /* number of extensions */
    int include_mode;      /* 1 for include, 0 for exclude */

} commc_watch_extension_filter_t;

/*
   create extension filter for monitoring
   specific file types with include/exclude
   logic for targeted change detection
*/

commc_watch_extension_filter_t* commc_watch_extension_filter_create(int include_mode);

/*
   destroy extension filter and free
   associated memory including extension
   strings and filter configuration
*/

void commc_watch_extension_filter_destroy(commc_watch_extension_filter_t* filter);

/*
   add file extension to filter with
   case sensitivity options for
   flexible file type filtering
*/

int commc_watch_extension_filter_add(commc_watch_extension_filter_t* filter,
                                    const char* extension);

/*
   remove file extension from filter
   and update filtering configuration
   for dynamic filter management
*/

int commc_watch_extension_filter_remove(commc_watch_extension_filter_t* filter,
                                       const char* extension);

/*
   test if path matches extension filter
   criteria for event filtering during
   monitoring operations
*/

int commc_watch_extension_filter_match(const commc_watch_extension_filter_t* filter,
                                      const char* path);

/*
   apply extension filter to watch context
   for automatic event filtering based
   on file type criteria
*/

int commc_watch_set_extension_filter(commc_watch_context_t* watch,
                                    const commc_watch_extension_filter_t* filter);

/* 
	==================================
      --- STATISTICS API ---
	==================================
*/

/*
   monitoring statistics structure for
   performance analysis and debugging
   of file system monitoring operations
*/

typedef struct {

    long events_processed;    /* total events processed */
    long events_filtered;     /* events filtered out */
    long callbacks_executed;  /* callback executions */

    long start_time;         /* monitoring start time */
    long last_event_time;    /* timestamp of last event */
    long total_runtime;      /* total monitoring time */

    long memory_used;        /* current memory usage */
    long peak_memory;        /* peak memory usage */

    int platform_errors;    /* platform-specific errors */
    int callback_errors;    /* callback execution errors */

} commc_watch_stats_t;

/*
   get monitoring statistics for
   performance analysis and debugging
   of file system monitoring operations
*/

commc_watch_stats_t* commc_watch_get_stats(const commc_watch_context_t* watch);

/*
   reset monitoring statistics for
   fresh performance measurement
   during monitoring sessions
*/

void commc_watch_reset_stats(commc_watch_context_t* watch);

/*
   print monitoring statistics in
   human-readable format for analysis
   and debugging purposes
*/

void commc_watch_print_stats(const commc_watch_stats_t* stats);

/*
   destroy statistics structure and
   free associated memory for
   cleanup after analysis
*/

void commc_watch_stats_destroy(commc_watch_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_WATCH_H */

/* 
	==================================
           --- EOF ---
	==================================
*/