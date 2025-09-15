/*
   ===================================
   C O M M O N - C
   NETWORKING UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- NET MODULE ---

    this module provides cross-platform networking
    abstractions for TCP and UDP socket communication.
    it aims to simplify common network programming tasks
    while maintaining C89 compliance.

    note: this module handles basic socket creation,
    connection, sending, and receiving. advanced features
    like non-blocking I/O or asynchronous operations
    may require further platform-specific extensions.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_NET_H
#define COMMC_NET_H

#include <stddef.h>       /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* socket types. */

typedef enum {

  COMMC_NET_TCP,         /* "Transmission Control Protocol" */
  COMMC_NET_UDP          /* "User Datagram Protocol" */

} commc_net_type_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* opaque socket handle. */

typedef int commc_socket_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_net_init()
	       ---
	       initializes the networking subsystem.
	       must be called once before any other net functions.
	       returns 1 on success, 0 on failure.

*/

int commc_net_init(void);

/*

         commc_net_shutdown()
	       ---
	       shuts down the networking subsystem.
	       must be called once at the end of program execution.

*/

void commc_net_shutdown(void);

/*

         commc_net_socket_create()
	       ---
	       creates a new socket of the specified type.
	       returns a valid socket handle on success, -1 on failure.

*/

commc_socket_t commc_net_socket_create(commc_net_type_t type);

/*

         commc_net_socket_close()
	       ---
	       closes an open socket.

*/

void commc_net_socket_close(commc_socket_t sock);

/*

         commc_net_bind()
	       ---
	       binds a socket to a specific port for listening
	       or sending UDP data. returns 1 on success, 0 on failure.

*/

int commc_net_bind(commc_socket_t sock, unsigned short port);

/*

         commc_net_listen()
	       ---
	       sets a TCP socket to listen for incoming connections.
	       returns 1 on success, 0 on failure.

*/

int commc_net_listen(commc_socket_t sock, int backlog);

/*

         commc_net_accept()
	       ---
	       accepts an incoming TCP connection on a listening socket.
	       returns a new socket handle for the connection, -1 on failure.

*/

commc_socket_t commc_net_accept(commc_socket_t listen_sock);

/*

         commc_net_connect()
	       ---
	       connects a TCP socket to a remote host and port.
	       returns 1 on success, 0 on failure.

*/

int commc_net_connect(commc_socket_t sock, const char* host, unsigned short port);

/*

         commc_net_send()
	       ---
	       sends data over a connected TCP socket.
	       returns number of bytes sent, -1 on error.

*/

int commc_net_send(commc_socket_t sock, const void* buffer, size_t len);

/*

         commc_net_recv()
	       ---
	       receives data from a connected TCP socket.
	       returns number of bytes received, -1 on error, 0 on disconnect.

*/

int commc_net_recv(commc_socket_t sock, void* buffer, size_t len);

/*

         commc_net_sendto()
	       ---
	       sends UDP data to a specific host and port.
	       returns number of bytes sent, -1 on error.

*/

int commc_net_sendto(commc_socket_t sock, const char* host, unsigned short port, const void* buffer, size_t len);

/*

         commc_net_recvfrom()
	       ---
	       receives UDP data, storing sender's address if needed.
	       returns number of bytes received, -1 on error.

*/

int commc_net_recvfrom(commc_socket_t sock, void* buffer, size_t len, char* out_host, size_t host_len, unsigned short* out_port);

#endif /* COMMC_NET_H */

/*
	==================================
             --- EOF ---
	==================================
*/
