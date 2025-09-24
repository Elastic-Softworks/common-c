/*
   ===================================
   C O M M O N - C
   ASYNCHRONOUS I/O OPERATIONS HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

         --- ASYNCHRONOUS I/O OPERATIONS ---

    cross-platform asynchronous I/O interface providing
    event-driven, non-blocking file and network operations.
    uses Windows I/O Completion Ports (IOCP) and Unix
    epoll/kqueue for high-performance async I/O.

    this module enables callback-based I/O operations,
    event loops, and scalable concurrent I/O processing
    suitable for servers and high-performance applications.

*/

#ifndef COMMC_ASYNC_H
#define COMMC_ASYNC_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  /* for size_t */
#include <time.h>    /* for timeout structures */

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/epoll.h>  /* for Linux epoll */
    #ifdef __APPLE__
        #include <sys/event.h>  /* for macOS/BSD kqueue */
    #endif
#endif

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_ASYNC_MAX_EVENTS      1024    /* MAX EVENTS PER POLL */
#define COMMC_ASYNC_DEFAULT_TIMEOUT 1000    /* DEFAULT TIMEOUT MS */
#define COMMC_ASYNC_INFINITE        -1      /* INFINITE TIMEOUT */

/*
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_async_event_type_t
	       ---
	       types of asynchronous events that can occur
	       during I/O operations.

*/

typedef enum {
    COMMC_ASYNC_EVENT_READ     = 0x01,  /* DATA AVAILABLE FOR READING */
    COMMC_ASYNC_EVENT_WRITE    = 0x02,  /* READY FOR WRITING */
    COMMC_ASYNC_EVENT_ACCEPT   = 0x04,  /* CONNECTION AVAILABLE */
    COMMC_ASYNC_EVENT_CONNECT  = 0x08,  /* CONNECTION COMPLETED */
    COMMC_ASYNC_EVENT_CLOSE    = 0x10,  /* CONNECTION CLOSED */
    COMMC_ASYNC_EVENT_ERROR    = 0x20,  /* ERROR OCCURRED */
    COMMC_ASYNC_EVENT_TIMEOUT  = 0x40   /* OPERATION TIMED OUT */
} commc_async_event_type_t;

/*

         commc_async_operation_type_t
	       ---
	       types of asynchronous operations that can
	       be queued for execution.

*/

typedef enum {
    COMMC_ASYNC_OP_READ      = 1,  /* READ OPERATION */
    COMMC_ASYNC_OP_WRITE     = 2,  /* WRITE OPERATION */
    COMMC_ASYNC_OP_ACCEPT    = 3,  /* ACCEPT OPERATION */
    COMMC_ASYNC_OP_CONNECT   = 4,  /* CONNECT OPERATION */
    COMMC_ASYNC_OP_SENDFILE  = 5,  /* SENDFILE OPERATION */
    COMMC_ASYNC_OP_RECV      = 6,  /* RECEIVE OPERATION */
    COMMC_ASYNC_OP_SEND      = 7   /* SEND OPERATION */
} commc_async_operation_type_t;

/*

         commc_async_result_t
	       ---
	       result structure containing information about
	       completed asynchronous operation.

*/

typedef struct {
    commc_async_operation_type_t  operation;     /* OPERATION TYPE */
    int                          handle;        /* FILE/SOCKET HANDLE */
    void*                        buffer;        /* DATA BUFFER */
    size_t                       bytes_transferred; /* BYTES PROCESSED */
    int                          error_code;    /* ERROR CODE (0 = SUCCESS) */
    void*                        user_data;     /* USER-PROVIDED DATA */
    
#ifdef _WIN32
    OVERLAPPED                   overlapped;    /* WINDOWS OVERLAPPED STRUCTURE */
#endif
} commc_async_result_t;

/*

         commc_async_callback_t
	       ---
	       callback function signature for async operation
	       completion notifications.

*/

typedef void (*commc_async_callback_t)(const commc_async_result_t* result);

/*

         commc_async_context_t
	       ---
	       asynchronous I/O context containing platform-specific
	       event handling mechanisms and operation tracking.

*/

typedef struct {
    int                      is_running;      /* EVENT LOOP STATUS */
    int                      max_events;      /* MAXIMUM EVENTS PER POLL */
    int                      timeout_ms;      /* POLLING TIMEOUT */
    commc_async_callback_t   default_callback; /* DEFAULT EVENT CALLBACK */
    
    /* operation tracking */
    void**                   operations;      /* PENDING OPERATIONS */
    size_t                   operation_count; /* NUMBER OF OPERATIONS */
    size_t                   operation_capacity; /* OPERATIONS ARRAY SIZE */
    
#ifdef _WIN32
    HANDLE                   completion_port; /* I/O COMPLETION PORT */
    HANDLE*                  threads;         /* WORKER THREADS */
    int                      thread_count;    /* NUMBER OF THREADS */
#elif defined(__linux__)
    int                      epoll_fd;        /* EPOLL FILE DESCRIPTOR */
    struct epoll_event*      events;          /* EVENT ARRAY */
#elif defined(__APPLE__) || defined(__FreeBSD__)
    int                      kqueue_fd;       /* KQUEUE FILE DESCRIPTOR */
    struct kevent*           events;          /* EVENT ARRAY */
    struct kevent*           changes;         /* CHANGE ARRAY */
    int                      change_count;    /* NUMBER OF CHANGES */
#endif
} commc_async_context_t;

/*

         commc_async_operation_t
	       ---
	       asynchronous operation structure containing
	       all information needed for async I/O request.

*/

typedef struct {
    commc_async_operation_type_t  type;        /* OPERATION TYPE */
    int                          handle;       /* FILE/SOCKET HANDLE */
    void*                        buffer;       /* DATA BUFFER */
    size_t                       buffer_size;  /* BUFFER SIZE */
    size_t                       offset;       /* FILE OFFSET */
    commc_async_callback_t       callback;     /* COMPLETION CALLBACK */
    void*                        user_data;    /* USER-PROVIDED DATA */
    
    /* connection info for network operations */
    void*                        address;      /* SOCKET ADDRESS */
    size_t                       address_size; /* ADDRESS SIZE */
    
    /* timing */
    int                          timeout_ms;   /* OPERATION TIMEOUT */
    
#ifdef _WIN32
    OVERLAPPED                   overlapped;   /* WINDOWS OVERLAPPED */
#endif
} commc_async_operation_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/*

         commc_async_context_create()
	       ---
	       creates new asynchronous I/O context with
	       platform-specific event handling setup.

*/

commc_async_context_t* commc_async_context_create(int max_events,
                                                  int timeout_ms);

/*

         commc_async_context_destroy()
	       ---
	       destroys async context and cleans up all
	       associated resources and pending operations.

*/

void commc_async_context_destroy(commc_async_context_t* ctx);

/*

         commc_async_context_set_callback()
	       ---
	       sets default callback function for operations
	       that don't specify their own callback.

*/

void commc_async_context_set_callback(commc_async_context_t* ctx,
                                     commc_async_callback_t callback);

/*

         commc_async_add_handle()
	       ---
	       adds file descriptor or handle to async context
	       for event monitoring. enables async operations
	       on the specified handle.

*/

int commc_async_add_handle(commc_async_context_t* ctx,
                          int handle,
                          commc_async_event_type_t events);

/*

         commc_async_remove_handle()
	       ---
	       removes handle from async context and cancels
	       any pending operations on that handle.

*/

int commc_async_remove_handle(commc_async_context_t* ctx,
                             int handle);

/*

         commc_async_modify_events()
	       ---
	       modifies event types being monitored for
	       specified handle in async context.

*/

int commc_async_modify_events(commc_async_context_t* ctx,
                             int handle,
                             commc_async_event_type_t events);

/*

         commc_async_read()
	       ---
	       initiates asynchronous read operation on
	       specified handle with callback notification.

*/

int commc_async_read(commc_async_context_t* ctx,
                    int handle,
                    void* buffer,
                    size_t buffer_size,
                    commc_async_callback_t callback,
                    void* user_data);

/*

         commc_async_write()
	       ---
	       initiates asynchronous write operation on
	       specified handle with callback notification.

*/

int commc_async_write(commc_async_context_t* ctx,
                     int handle,
                     const void* buffer,
                     size_t buffer_size,
                     commc_async_callback_t callback,
                     void* user_data);

/*

         commc_async_read_file()
	       ---
	       initiates asynchronous file read operation
	       at specified offset with callback notification.

*/

int commc_async_read_file(commc_async_context_t* ctx,
                         int handle,
                         void* buffer,
                         size_t buffer_size,
                         size_t offset,
                         commc_async_callback_t callback,
                         void* user_data);

/*

         commc_async_write_file()
	       ---
	       initiates asynchronous file write operation
	       at specified offset with callback notification.

*/

int commc_async_write_file(commc_async_context_t* ctx,
                          int handle,
                          const void* buffer,
                          size_t buffer_size,
                          size_t offset,
                          commc_async_callback_t callback,
                          void* user_data);

/*

         commc_async_accept()
	       ---
	       initiates asynchronous accept operation on
	       listening socket with callback notification.

*/

int commc_async_accept(commc_async_context_t* ctx,
                      int listen_handle,
                      int* accept_handle,
                      void* address_buffer,
                      size_t address_size,
                      commc_async_callback_t callback,
                      void* user_data);

/*

         commc_async_connect()
	       ---
	       initiates asynchronous connect operation to
	       specified address with callback notification.

*/

int commc_async_connect(commc_async_context_t* ctx,
                       int handle,
                       const void* address,
                       size_t address_size,
                       commc_async_callback_t callback,
                       void* user_data);

/*

         commc_async_sendfile()
	       ---
	       initiates asynchronous sendfile operation
	       (zero-copy file transmission) with callback.

*/

int commc_async_sendfile(commc_async_context_t* ctx,
                        int out_handle,
                        int in_handle,
                        size_t offset,
                        size_t count,
                        commc_async_callback_t callback,
                        void* user_data);

/*

         commc_async_cancel()
	       ---
	       cancels pending asynchronous operation on
	       specified handle. callback may still be called
	       with cancellation status.

*/

int commc_async_cancel(commc_async_context_t* ctx,
                      int handle);

/*

         commc_async_cancel_all()
	       ---
	       cancels all pending asynchronous operations
	       in the context.

*/

int commc_async_cancel_all(commc_async_context_t* ctx);

/*

         commc_async_poll()
	       ---
	       polls for completed operations and executes
	       callbacks. returns number of events processed
	       or -1 on error.

*/

int commc_async_poll(commc_async_context_t* ctx,
                    int timeout_ms);

/*

         commc_async_poll_once()
	       ---
	       polls for events once without blocking.
	       returns immediately with count of events
	       processed.

*/

int commc_async_poll_once(commc_async_context_t* ctx);

/*

         commc_async_run()
	       ---
	       runs event loop continuously until stopped.
	       processes events and executes callbacks as
	       they complete.

*/

void commc_async_run(commc_async_context_t* ctx);

/*

         commc_async_stop()
	       ---
	       stops running event loop gracefully.
	       allows current operations to complete.

*/

void commc_async_stop(commc_async_context_t* ctx);

/*

         commc_async_is_running()
	       ---
	       checks if event loop is currently running.

*/

int commc_async_is_running(const commc_async_context_t* ctx);

/*

         commc_async_get_pending_count()
	       ---
	       returns number of pending operations in
	       the async context.

*/

size_t commc_async_get_pending_count(const commc_async_context_t* ctx);

/*

         commc_async_set_timeout()
	       ---
	       sets default timeout for polling operations
	       in the async context.

*/

void commc_async_set_timeout(commc_async_context_t* ctx,
                            int timeout_ms);

/*

         commc_async_get_timeout()
	       ---
	       gets current default timeout value for
	       the async context.

*/

int commc_async_get_timeout(const commc_async_context_t* ctx);

/*

         commc_async_operation_create()
	       ---
	       creates new async operation structure with
	       specified parameters.

*/

commc_async_operation_t* commc_async_operation_create(commc_async_operation_type_t type,
                                                     int handle,
                                                     void* buffer,
                                                     size_t buffer_size);

/*

         commc_async_operation_destroy()
	       ---
	       destroys async operation structure and
	       frees associated memory.

*/

void commc_async_operation_destroy(commc_async_operation_t* op);

/*

         commc_async_submit_operation()
	       ---
	       submits operation to async context for
	       execution. alternative to specific async_* calls.

*/

int commc_async_submit_operation(commc_async_context_t* ctx,
                                commc_async_operation_t* op);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_ASYNC_H */

/*
	==================================
             --- EOF ---
	==================================
*/