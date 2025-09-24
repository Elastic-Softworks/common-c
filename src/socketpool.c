/* 	
   ===================================
   SOCKETPOOL.C
   SOCKET CONNECTION POOLING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    socket connection pooling implementation for
	    efficient reuse and management of TCP/UDP sockets.
	    provides pool creation, acquisition, release,
	    and cleanup. C89 compliant and cross-platform.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commc/socketpool.h"
#include "commc/socket.h"
#include "commc/error.h"

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
                                      int                  max_sockets) {
    commc_socketpool_t* new_pool;
    int i;
    if (!pool || !hostname || port <= 0 || max_sockets <= 0 || max_sockets > COMMC_SOCKETPOOL_MAX_SOCKETS) {
        return COMMC_ERROR_INVALID_ARGUMENT;
    }
    new_pool = (commc_socketpool_t*)malloc(sizeof(commc_socketpool_t));
    if (!new_pool) {
        return COMMC_MEMORY_ERROR;
    }
    memset(new_pool, 0, sizeof(commc_socketpool_t));
    strncpy(new_pool->hostname, hostname, sizeof(new_pool->hostname)-1);
    new_pool->port = port;
    new_pool->pool_type = pool_type;
    new_pool->max_sockets = max_sockets;
    new_pool->socket_count = 0;
    for (i = 0; i < max_sockets; i++) {
        new_pool->sockets[i] = NULL;
        new_pool->in_use[i] = 0;
    }
    *pool = new_pool;
    return COMMC_SUCCESS;
}

/*
         commc_socketpool_destroy()
	       ---
	       destroys socket pool and closes all
	       managed sockets.
*/
void commc_socketpool_destroy(commc_socketpool_t* pool) {
    int i;
    if (!pool) return;
    for (i = 0; i < pool->max_sockets; i++) {
        if (pool->sockets[i]) {
            commc_socket_destroy(pool->sockets[i]);
            pool->sockets[i] = NULL;
        }
    }
    free(pool);
}

/*
         commc_socketpool_acquire()
	       ---
	       acquires an available socket from the pool
	       for use. returns pointer to socket or NULL.
*/
commc_socket_t* commc_socketpool_acquire(commc_socketpool_t* pool) {
    int i;
    if (!pool) return NULL;
    for (i = 0; i < pool->max_sockets; i++) {
        if (!pool->in_use[i]) {
            if (!pool->sockets[i]) {
                /* Create new socket if not present */
                pool->sockets[i] = (commc_socket_t*)malloc(sizeof(commc_socket_t));
                if (!pool->sockets[i]) return NULL;
                memset(pool->sockets[i], 0, sizeof(commc_socket_t));
                pool->sockets[i]->type = pool->pool_type == 0 ? COMMC_SOCKET_TYPE_TCP : COMMC_SOCKET_TYPE_UDP;
                if (pool->pool_type == 0) {
                    /* TCP */
                    if (commc_socket_connect_hostname(pool->sockets[i], pool->hostname, NULL, COMMC_SOCKET_DEFAULT_TIMEOUT) != COMMC_SUCCESS) {
                        commc_socket_destroy(pool->sockets[i]);
                        pool->sockets[i] = NULL;
                        continue;
                    }
                } else {
                    /* UDP: just initialize type, no connect needed */
                }
                pool->socket_count++;
            }
            pool->in_use[i] = 1;
            return pool->sockets[i];
        }
    }
    return NULL;
}

/*
         commc_socketpool_release()
	       ---
	       releases a socket back to the pool after use.
*/
commc_error_t commc_socketpool_release(commc_socketpool_t* pool,
                                       commc_socket_t*     socket) {
    int i;
    if (!pool || !socket) return COMMC_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < pool->max_sockets; i++) {
        if (pool->sockets[i] == socket) {
            pool->in_use[i] = 0;
            return COMMC_SUCCESS;
        }
    }
    return COMMC_ERROR_INVALID_ARGUMENT;
}

/*
         commc_socketpool_get_count()
	       ---
	       returns the number of sockets currently
	       managed by the pool.
*/
int commc_socketpool_get_count(const commc_socketpool_t* pool) {
    if (!pool) return 0;
    return pool->socket_count;
}

/* 
	==================================
             --- EOF ---
	==================================
*/
