/* 	
   ===================================
   SOCKETPOOL.H
   SOCKET CONNECTION POOLING HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    socket connection pooling system for efficient
	    reuse and management of TCP/UDP socket resources.
	    provides thread-safe pool creation, acquisition,
	    release, and cleanup for high-performance networking.
	       
	    designed for educational clarity, C89 compliance,
	    and cross-platform compatibility.

*/

#ifndef COMMC_SOCKETPOOL_H
#define COMMC_SOCKETPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "commc/socket.h"
#include "commc/error.h"

#define COMMC_SOCKETPOOL_MAX_SOCKETS 128

/*
         commc_socketpool_t
	       ---
	       socket pool structure for managing
	       multiple reusable socket connections.
*/

typedef struct {
    commc_socket_t* sockets[COMMC_SOCKETPOOL_MAX_SOCKETS];
    int            in_use[COMMC_SOCKETPOOL_MAX_SOCKETS];
    int            socket_count;
    int            max_sockets;
    int            pool_type; /* 0: TCP, 1: UDP */
    int            port;
    char           hostname[256];
} commc_socketpool_t;

/*
         commc_socketpool_create()
	       ---
	       creates and initializes a socket pool
	       for specified host, port, and type.
*/
commc_error_t commc_socketpool_create(commc_socketpool_t** pool,
                                      const char*          hostname,
                                      int                  port,
                                      int                  pool_type,
                                      int                  max_sockets);

/*
         commc_socketpool_destroy()
	       ---
	       destroys socket pool and closes all
	       managed sockets.
*/
void commc_socketpool_destroy(commc_socketpool_t* pool);

/*
         commc_socketpool_acquire()
	       ---
	       acquires an available socket from the pool
	       for use. returns pointer to socket or NULL.
*/
commc_socket_t* commc_socketpool_acquire(commc_socketpool_t* pool);

/*
         commc_socketpool_release()
	       ---
	       releases a socket back to the pool after use.
*/
commc_error_t commc_socketpool_release(commc_socketpool_t* pool,
                                       commc_socket_t*     socket);

/*
         commc_socketpool_get_count()
	       ---
	       returns the number of sockets currently
	       managed by the pool.
*/
int commc_socketpool_get_count(const commc_socketpool_t* pool);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_SOCKETPOOL_H */

/* 
	==================================
             --- EOF ---
	==================================
*/
