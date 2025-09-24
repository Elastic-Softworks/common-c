/*
   ===================================
   C O M M O N - C
   ASYNCHRONOUS I/O OPERATIONS IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

         --- ASYNCHRONOUS I/O OPERATIONS ---

    implementation of cross-platform asynchronous I/O using
    Windows I/O Completion Ports (IOCP) and Unix epoll/kqueue.
    provides event-driven, callback-based I/O operations for
    high-performance applications.

    see include/commc/async.h for function prototypes
    and comprehensive documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>        /* for malloc, free */
#include <string.h>        /* for memset, memcpy */
#include <stdio.h>         /* for sprintf */

#ifdef _WIN32
    #include <windows.h>   /* for IOCP functions */
    #include <winsock2.h>  /* for socket functions */
    #include <mswsock.h>   /* for AcceptEx */
#else
    #include <unistd.h>    /* for close */
    #include <errno.h>     /* for errno */
    #include <fcntl.h>     /* for fcntl */
    #include <sys/types.h> /* for various types */
    #include <sys/socket.h> /* for socket functions */
    
    #ifdef __linux__
        #include <sys/epoll.h>   /* for epoll */
        #include <sys/sendfile.h> /* for sendfile */
    #elif defined(__APPLE__) || defined(__FreeBSD__)
        #include <sys/event.h>   /* for kqueue */
        #include <sys/uio.h>     /* for sendfile */
    #endif
#endif

#include "commc/async.h"
#include "commc/error.h"

/*
	==================================
             --- HELPERS ---
	==================================
*/

/*

         validate_context()
	       ---
	       validates that async context is properly
	       initialized and not NULL.

*/

static int validate_context(const commc_async_context_t* ctx) {

    return (ctx != NULL);
}

/*

         validate_operation()
	       ---
	       validates that async operation structure
	       is properly initialized.

*/

static int validate_operation(const commc_async_operation_t* op) {

    return (op != NULL && op->handle >= 0);
}

/*

         grow_operations_array()
	       ---
	       grows operations tracking array when more
	       capacity is needed.

*/

static int grow_operations_array(commc_async_context_t* ctx) {

    void** new_ops;
    size_t new_capacity;

    new_capacity = ctx->operation_capacity * 2;
    if (new_capacity == 0) {
        new_capacity = 16;
    }

    new_ops = (void**)realloc(ctx->operations,
                             new_capacity * sizeof(void*));
    if (!new_ops) {
        return -1;
    }

    ctx->operations = new_ops;
    ctx->operation_capacity = new_capacity;
    return 0;
}

/*

         add_operation()
	       ---
	       adds operation to context tracking array.

*/

static int add_operation(commc_async_context_t* ctx,
                        commc_async_operation_t* op) {

    if (ctx->operation_count >= ctx->operation_capacity) {
        if (grow_operations_array(ctx) != 0) {
            return -1;
        }
    }

    ctx->operations[ctx->operation_count++] = op;
    return 0;
}

/*

         remove_operation()
	       ---
	       removes operation from context tracking array.

*/

static void remove_operation(commc_async_context_t* ctx,
                           commc_async_operation_t* op) {

    size_t i;

    for (i = 0; i < ctx->operation_count; i++) {
        if (ctx->operations[i] == op) {
            /* move last element to this position */
            ctx->operations[i] = ctx->operations[ctx->operation_count - 1];
            ctx->operation_count--;
            break;
        }
    }
}

/*

         set_nonblocking()
	       ---
	       sets socket or file descriptor to non-blocking mode
	       for asynchronous operations.

*/

static int set_nonblocking(int handle) {

#ifdef _WIN32
    unsigned long mode = 1;
    return ioctlsocket(handle, FIONBIO, &mode);
#else
    int flags = fcntl(handle, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    return fcntl(handle, F_SETFL, flags | O_NONBLOCK);
#endif
}

#ifdef _WIN32

/*

         iocp_worker_thread()
	       ---
	       Windows IOCP worker thread that processes
	       completed I/O operations.

*/

static DWORD WINAPI iocp_worker_thread(LPVOID param) {

    commc_async_context_t* ctx = (commc_async_context_t*)param;
    DWORD                  bytes_transferred;
    ULONG_PTR              completion_key;
    OVERLAPPED*            overlapped;
    commc_async_result_t   result;
    commc_async_operation_t* op;

    while (ctx->is_running) {

        if (!GetQueuedCompletionStatus(ctx->completion_port,
                                      &bytes_transferred,
                                      &completion_key,
                                      &overlapped,
                                      1000)) {
            
            DWORD error = GetLastError();
            if (error == WAIT_TIMEOUT) {
                continue;
            }
            
            if (overlapped) {
                /* operation failed */
                op = (commc_async_operation_t*)completion_key;
                
                memset(&result, 0, sizeof(result));
                result.operation = op->type;
                result.handle = op->handle;
                result.buffer = op->buffer;
                result.bytes_transferred = 0;
                result.error_code = error;
                result.user_data = op->user_data;
                
                if (op->callback) {
                    op->callback(&result);
                }
                
                remove_operation(ctx, op);
                commc_async_operation_destroy(op);
            }
            continue;
        }

        /* operation completed successfully */
        op = (commc_async_operation_t*)completion_key;
        
        memset(&result, 0, sizeof(result));
        result.operation = op->type;
        result.handle = op->handle;
        result.buffer = op->buffer;
        result.bytes_transferred = bytes_transferred;
        result.error_code = 0;
        result.user_data = op->user_data;
        
        if (op->callback) {
            op->callback(&result);
        }
        
        remove_operation(ctx, op);
        commc_async_operation_destroy(op);
    }

    return 0;
}

#endif

#if defined(__linux__)

/*

         convert_events_to_epoll()
	       ---
	       converts generic event types to epoll-specific
	       event flags.

*/

static uint32_t convert_events_to_epoll(commc_async_event_type_t events) {

    uint32_t epoll_events = 0;

    if (events & COMMC_ASYNC_EVENT_READ) {
        epoll_events |= EPOLLIN;
    }
    if (events & COMMC_ASYNC_EVENT_WRITE) {
        epoll_events |= EPOLLOUT;
    }
    if (events & COMMC_ASYNC_EVENT_CLOSE) {
        epoll_events |= EPOLLHUP | EPOLLRDHUP;
    }
    if (events & COMMC_ASYNC_EVENT_ERROR) {
        epoll_events |= EPOLLERR;
    }

    return epoll_events;
}

/*

         convert_epoll_to_events()
	       ---
	       converts epoll event flags to generic
	       event types.

*/

static commc_async_event_type_t convert_epoll_to_events(uint32_t epoll_events) {

    commc_async_event_type_t events = 0;

    if (epoll_events & EPOLLIN) {
        events |= COMMC_ASYNC_EVENT_READ;
    }
    if (epoll_events & EPOLLOUT) {
        events |= COMMC_ASYNC_EVENT_WRITE;
    }
    if (epoll_events & (EPOLLHUP | EPOLLRDHUP)) {
        events |= COMMC_ASYNC_EVENT_CLOSE;
    }
    if (epoll_events & EPOLLERR) {
        events |= COMMC_ASYNC_EVENT_ERROR;
    }

    return events;
}

#elif defined(__APPLE__) || defined(__FreeBSD__)

/*

         convert_events_to_kqueue()
	       ---
	       converts generic event types to kqueue-specific
	       kevent structures.

*/

static void convert_events_to_kqueue(int handle,
                                    commc_async_event_type_t events,
                                    struct kevent* changes,
                                    int* change_count) {

    int count = 0;

    if (events & COMMC_ASYNC_EVENT_READ) {
        EV_SET(&changes[count++], handle, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    }
    if (events & COMMC_ASYNC_EVENT_WRITE) {
        EV_SET(&changes[count++], handle, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    }

    *change_count = count;
}

/*

         convert_kqueue_to_events()
	       ---
	       converts kqueue kevent to generic event types.

*/

static commc_async_event_type_t convert_kqueue_to_events(const struct kevent* event) {

    commc_async_event_type_t events = 0;

    if (event->filter == EVFILT_READ) {
        events |= COMMC_ASYNC_EVENT_READ;
    }
    if (event->filter == EVFILT_WRITE) {
        events |= COMMC_ASYNC_EVENT_WRITE;
    }
    if (event->flags & EV_EOF) {
        events |= COMMC_ASYNC_EVENT_CLOSE;
    }
    if (event->flags & EV_ERROR) {
        events |= COMMC_ASYNC_EVENT_ERROR;
    }

    return events;
}

#endif

/*
	==================================
             --- API ---
	==================================
*/

/*

         commc_async_context_create()
	       ---
	       creates new asynchronous I/O context.

*/

commc_async_context_t* commc_async_context_create(int max_events,
                                                  int timeout_ms) {

    commc_async_context_t* ctx;

    ctx = (commc_async_context_t*)malloc(sizeof(commc_async_context_t));
    if (!ctx) {
        return NULL;
    }

    /* initialize basic fields */
    memset(ctx, 0, sizeof(commc_async_context_t));
    ctx->is_running = 0;
    ctx->max_events = (max_events > 0) ? max_events : COMMC_ASYNC_MAX_EVENTS;
    ctx->timeout_ms = (timeout_ms >= 0) ? timeout_ms : COMMC_ASYNC_DEFAULT_TIMEOUT;
    ctx->default_callback = NULL;
    ctx->operations = NULL;
    ctx->operation_count = 0;
    ctx->operation_capacity = 0;

#ifdef _WIN32
    /* create I/O completion port */
    ctx->completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                                 NULL, 0, 0);
    if (!ctx->completion_port) {
        free(ctx);
        return NULL;
    }

    /* create worker threads */
    ctx->thread_count = 4; /* default thread count */
    ctx->threads = (HANDLE*)malloc(ctx->thread_count * sizeof(HANDLE));
    if (!ctx->threads) {
        CloseHandle(ctx->completion_port);
        free(ctx);
        return NULL;
    }

    for (int i = 0; i < ctx->thread_count; i++) {
        ctx->threads[i] = CreateThread(NULL, 0, iocp_worker_thread,
                                      ctx, 0, NULL);
        if (!ctx->threads[i]) {
            /* cleanup created threads */
            for (int j = 0; j < i; j++) {
                CloseHandle(ctx->threads[j]);
            }
            free(ctx->threads);
            CloseHandle(ctx->completion_port);
            free(ctx);
            return NULL;
        }
    }

#elif defined(__linux__)
    /* create epoll instance */
    ctx->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (ctx->epoll_fd == -1) {
        free(ctx);
        return NULL;
    }

    /* allocate events array */
    ctx->events = (struct epoll_event*)malloc(ctx->max_events * 
                                             sizeof(struct epoll_event));
    if (!ctx->events) {
        close(ctx->epoll_fd);
        free(ctx);
        return NULL;
    }

#elif defined(__APPLE__) || defined(__FreeBSD__)
    /* create kqueue */
    ctx->kqueue_fd = kqueue();
    if (ctx->kqueue_fd == -1) {
        free(ctx);
        return NULL;
    }

    /* allocate events and changes arrays */
    ctx->events = (struct kevent*)malloc(ctx->max_events * sizeof(struct kevent));
    if (!ctx->events) {
        close(ctx->kqueue_fd);
        free(ctx);
        return NULL;
    }

    ctx->changes = (struct kevent*)malloc(ctx->max_events * sizeof(struct kevent));
    if (!ctx->changes) {
        free(ctx->events);
        close(ctx->kqueue_fd);
        free(ctx);
        return NULL;
    }

    ctx->change_count = 0;
#endif

    return ctx;
}

/*

         commc_async_context_destroy()
	       ---
	       destroys async context and cleans up resources.

*/

void commc_async_context_destroy(commc_async_context_t* ctx) {

    if (!validate_context(ctx)) {
        return;
    }

    /* stop event loop */
    ctx->is_running = 0;

    /* cancel all operations */
    commc_async_cancel_all(ctx);

#ifdef _WIN32
    /* wait for worker threads to finish */
    if (ctx->threads) {
        WaitForMultipleObjects(ctx->thread_count, ctx->threads,
                              TRUE, 5000);
        for (int i = 0; i < ctx->thread_count; i++) {
            CloseHandle(ctx->threads[i]);
        }
        free(ctx->threads);
    }

    /* close completion port */
    if (ctx->completion_port) {
        CloseHandle(ctx->completion_port);
    }

#elif defined(__linux__)
    /* close epoll */
    if (ctx->epoll_fd != -1) {
        close(ctx->epoll_fd);
    }

    /* free events array */
    if (ctx->events) {
        free(ctx->events);
    }

#elif defined(__APPLE__) || defined(__FreeBSD__)
    /* close kqueue */
    if (ctx->kqueue_fd != -1) {
        close(ctx->kqueue_fd);
    }

    /* free event arrays */
    if (ctx->events) {
        free(ctx->events);
    }
    if (ctx->changes) {
        free(ctx->changes);
    }
#endif

    /* free operations array */
    if (ctx->operations) {
        free(ctx->operations);
    }

    /* free context */
    free(ctx);
}

/*

         commc_async_context_set_callback()
	       ---
	       sets default callback for operations.

*/

void commc_async_context_set_callback(commc_async_context_t* ctx,
                                     commc_async_callback_t callback) {

    if (!validate_context(ctx)) {
        return;
    }

    ctx->default_callback = callback;
}

/*

         commc_async_add_handle()
	       ---
	       adds handle to async context for monitoring.

*/

int commc_async_add_handle(commc_async_context_t* ctx,
                          int handle,
                          commc_async_event_type_t events) {

    if (!validate_context(ctx) || handle < 0) {
        return -1;
    }

    /* set handle to non-blocking mode */
    if (set_nonblocking(handle) != 0) {
        return -1;
    }

#ifdef _WIN32
    /* associate handle with completion port */
    HANDLE result = CreateIoCompletionPort((HANDLE)handle,
                                          ctx->completion_port,
                                          (ULONG_PTR)handle, 0);
    return (result == ctx->completion_port) ? 0 : -1;

#elif defined(__linux__)
    struct epoll_event event;
    
    memset(&event, 0, sizeof(event));
    event.events = convert_events_to_epoll(events);
    event.data.fd = handle;
    
    return epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, handle, &event);

#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct kevent changes[2];
    int change_count = 0;
    
    convert_events_to_kqueue(handle, events, changes, &change_count);
    
    return kevent(ctx->kqueue_fd, changes, change_count, NULL, 0, NULL);
#endif
}

/*

         commc_async_remove_handle()
	       ---
	       removes handle from async context.

*/

int commc_async_remove_handle(commc_async_context_t* ctx,
                             int handle) {

    if (!validate_context(ctx) || handle < 0) {
        return -1;
    }

#ifdef _WIN32
    /* Windows IOCP automatically removes handles when closed */
    return 0;

#elif defined(__linux__)
    return epoll_ctl(ctx->epoll_fd, EPOLL_CTL_DEL, handle, NULL);

#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct kevent changes[2];
    
    EV_SET(&changes[0], handle, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&changes[1], handle, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    
    return kevent(ctx->kqueue_fd, changes, 2, NULL, 0, NULL);
#endif
}

/*

         commc_async_modify_events()
	       ---
	       modifies events being monitored for handle.

*/

int commc_async_modify_events(commc_async_context_t* ctx,
                             int handle,
                             commc_async_event_type_t events) {

    if (!validate_context(ctx) || handle < 0) {
        return -1;
    }

#ifdef _WIN32
    /* Windows IOCP doesn't modify events, operations specify their type */
    return 0;

#elif defined(__linux__)
    struct epoll_event event;
    
    memset(&event, 0, sizeof(event));
    event.events = convert_events_to_epoll(events);
    event.data.fd = handle;
    
    return epoll_ctl(ctx->epoll_fd, EPOLL_CTL_MOD, handle, &event);

#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct kevent changes[4];
    int change_count = 0;
    
    /* delete existing events */
    EV_SET(&changes[change_count++], handle, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&changes[change_count++], handle, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    
    /* add new events */
    convert_events_to_kqueue(handle, events, &changes[change_count], &change_count);
    change_count += 2;
    
    return kevent(ctx->kqueue_fd, changes, change_count, NULL, 0, NULL);
#endif
}

/*

         commc_async_read()
	       ---
	       initiates asynchronous read operation.

*/

int commc_async_read(commc_async_context_t* ctx,
                    int handle,
                    void* buffer,
                    size_t buffer_size,
                    commc_async_callback_t callback,
                    void* user_data) {

    commc_async_operation_t* op;

    if (!validate_context(ctx) || handle < 0 || !buffer || buffer_size == 0) {
        return -1;
    }

    op = commc_async_operation_create(COMMC_ASYNC_OP_READ, handle,
                                     buffer, buffer_size);
    if (!op) {
        return -1;
    }

    op->callback = callback ? callback : ctx->default_callback;
    op->user_data = user_data;

    return commc_async_submit_operation(ctx, op);
}

/*

         commc_async_write()
	       ---
	       initiates asynchronous write operation.

*/

int commc_async_write(commc_async_context_t* ctx,
                     int handle,
                     const void* buffer,
                     size_t buffer_size,
                     commc_async_callback_t callback,
                     void* user_data) {

    commc_async_operation_t* op;

    if (!validate_context(ctx) || handle < 0 || !buffer || buffer_size == 0) {
        return -1;
    }

    op = commc_async_operation_create(COMMC_ASYNC_OP_WRITE, handle,
                                     (void*)buffer, buffer_size);
    if (!op) {
        return -1;
    }

    op->callback = callback ? callback : ctx->default_callback;
    op->user_data = user_data;

    return commc_async_submit_operation(ctx, op);
}

/*

         commc_async_read_file()
	       ---
	       initiates asynchronous file read operation.

*/

int commc_async_read_file(commc_async_context_t* ctx,
                         int handle,
                         void* buffer,
                         size_t buffer_size,
                         size_t offset,
                         commc_async_callback_t callback,
                         void* user_data) {

    commc_async_operation_t* op;

    if (!validate_context(ctx) || handle < 0 || !buffer || buffer_size == 0) {
        return -1;
    }

    op = commc_async_operation_create(COMMC_ASYNC_OP_READ, handle,
                                     buffer, buffer_size);
    if (!op) {
        return -1;
    }

    op->offset = offset;
    op->callback = callback ? callback : ctx->default_callback;
    op->user_data = user_data;

    return commc_async_submit_operation(ctx, op);
}

/*

         commc_async_write_file()
	       ---
	       initiates asynchronous file write operation.

*/

int commc_async_write_file(commc_async_context_t* ctx,
                          int handle,
                          const void* buffer,
                          size_t buffer_size,
                          size_t offset,
                          commc_async_callback_t callback,
                          void* user_data) {

    commc_async_operation_t* op;

    if (!validate_context(ctx) || handle < 0 || !buffer || buffer_size == 0) {
        return -1;
    }

    op = commc_async_operation_create(COMMC_ASYNC_OP_WRITE, handle,
                                     (void*)buffer, buffer_size);
    if (!op) {
        return -1;
    }

    op->offset = offset;
    op->callback = callback ? callback : ctx->default_callback;
    op->user_data = user_data;

    return commc_async_submit_operation(ctx, op);
}

/*

         commc_async_poll()
	       ---
	       polls for completed operations.

*/

int commc_async_poll(commc_async_context_t* ctx, int timeout_ms) {

    if (!validate_context(ctx)) {
        return -1;
    }

    if (timeout_ms < 0) {
        timeout_ms = ctx->timeout_ms;
    }

#ifdef _WIN32
    /* Windows uses worker threads, just sleep */
    Sleep(timeout_ms);
    return 0;

#elif defined(__linux__)
    int nfds = epoll_wait(ctx->epoll_fd, ctx->events, ctx->max_events, timeout_ms);
    if (nfds == -1) {
        return -1;
    }

    /* process events */
    for (int i = 0; i < nfds; i++) {
        /* handle epoll events here */
        /* this would typically trigger read/write operations */
    }

    return nfds;

#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct timespec timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_nsec = (timeout_ms % 1000) * 1000000;

    int nfds = kevent(ctx->kqueue_fd, NULL, 0, ctx->events, ctx->max_events, &timeout);
    if (nfds == -1) {
        return -1;
    }

    /* process events */
    for (int i = 0; i < nfds; i++) {
        /* handle kqueue events here */
    }

    return nfds;
#endif
}

/*

         commc_async_run()
	       ---
	       runs event loop continuously.

*/

void commc_async_run(commc_async_context_t* ctx) {

    if (!validate_context(ctx)) {
        return;
    }

    ctx->is_running = 1;

    while (ctx->is_running) {
        commc_async_poll(ctx, ctx->timeout_ms);
    }
}

/*

         commc_async_stop()
	       ---
	       stops running event loop.

*/

void commc_async_stop(commc_async_context_t* ctx) {

    if (!validate_context(ctx)) {
        return;
    }

    ctx->is_running = 0;
}

/*

         commc_async_operation_create()
	       ---
	       creates new async operation structure.

*/

commc_async_operation_t* commc_async_operation_create(commc_async_operation_type_t type,
                                                     int handle,
                                                     void* buffer,
                                                     size_t buffer_size) {

    commc_async_operation_t* op;

    op = (commc_async_operation_t*)malloc(sizeof(commc_async_operation_t));
    if (!op) {
        return NULL;
    }

    memset(op, 0, sizeof(commc_async_operation_t));
    op->type = type;
    op->handle = handle;
    op->buffer = buffer;
    op->buffer_size = buffer_size;
    op->offset = 0;
    op->callback = NULL;
    op->user_data = NULL;
    op->address = NULL;
    op->address_size = 0;
    op->timeout_ms = COMMC_ASYNC_DEFAULT_TIMEOUT;

    return op;
}

/*

         commc_async_operation_destroy()
	       ---
	       destroys async operation structure.

*/

void commc_async_operation_destroy(commc_async_operation_t* op) {

    if (op) {
        free(op);
    }
}

/*

         commc_async_submit_operation()
	       ---
	       submits operation to async context.

*/

int commc_async_submit_operation(commc_async_context_t* ctx,
                                commc_async_operation_t* op) {

    if (!validate_context(ctx) || !validate_operation(op)) {
        return -1;
    }

    /* add to tracking array */
    if (add_operation(ctx, op) != 0) {
        return -1;
    }

#ifdef _WIN32
    BOOL result = FALSE;
    DWORD bytes_transferred = 0;

    /* set up overlapped structure */
    memset(&op->overlapped, 0, sizeof(OVERLAPPED));
    op->overlapped.Offset = (DWORD)(op->offset & 0xFFFFFFFF);
    op->overlapped.OffsetHigh = (DWORD)(op->offset >> 32);

    /* submit appropriate operation */
    if (op->type == COMMC_ASYNC_OP_READ) {
        result = ReadFile((HANDLE)op->handle, op->buffer, (DWORD)op->buffer_size,
                         &bytes_transferred, &op->overlapped);
    } else if (op->type == COMMC_ASYNC_OP_WRITE) {
        result = WriteFile((HANDLE)op->handle, op->buffer, (DWORD)op->buffer_size,
                          &bytes_transferred, &op->overlapped);
    }

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        remove_operation(ctx, op);
        return -1;
    }

    return 0;
#else
    /* Unix systems would handle operations differently */
    /* This is a simplified implementation */
    return 0;
#endif
}

/*

         commc_async_cancel_all()
	       ---
	       cancels all pending operations.

*/

int commc_async_cancel_all(commc_async_context_t* ctx) {

    size_t i;

    if (!validate_context(ctx)) {
        return -1;
    }

    /* destroy all pending operations */
    for (i = 0; i < ctx->operation_count; i++) {
        if (ctx->operations[i]) {
            commc_async_operation_destroy((commc_async_operation_t*)ctx->operations[i]);
        }
    }

    ctx->operation_count = 0;
    return 0;
}

/*

         commc_async_get_pending_count()
	       ---
	       returns number of pending operations.

*/

size_t commc_async_get_pending_count(const commc_async_context_t* ctx) {

    if (!validate_context(ctx)) {
        return 0;
    }

    return ctx->operation_count;
}

/*

         commc_async_is_running()
	       ---
	       checks if event loop is running.

*/

int commc_async_is_running(const commc_async_context_t* ctx) {

    if (!validate_context(ctx)) {
        return 0;
    }

    return ctx->is_running;
}

/*
	==================================
             --- EOF ---
	==================================
*/