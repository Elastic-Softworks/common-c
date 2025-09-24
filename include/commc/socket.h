/* 	
   ===================================
   S O C K E T . H
   CROSS-PLATFORM SOCKET WRAPPER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    cross-platform Berkeley sockets wrapper providing
	    unified interface for TCP/UDP networking operations.
	    abstracts platform differences while maintaining
	    educational clarity and error handling consistency.
	       
	    supports both client and server operations with
	    comprehensive error reporting and timeout management.
	    designed for robust network application development.

*/

#ifndef COMMC_SOCKET_H
#define COMMC_SOCKET_H

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "error.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

#define COMMC_SOCKET_MAX_HOSTNAME_LENGTH    256
#define COMMC_SOCKET_MAX_SERVICE_LENGTH     64
#define COMMC_SOCKET_MAX_BACKLOG           128
#define COMMC_SOCKET_BUFFER_SIZE          8192
#define COMMC_SOCKET_DEFAULT_TIMEOUT        30

#ifdef _WIN32
#define COMMC_INVALID_SOCKET (-1)
#define COMMC_SOCKET_ERROR   SOCKET_ERROR
#else
#define COMMC_INVALID_SOCKET (-1)
#define COMMC_SOCKET_ERROR   (-1)
#endif

/* 
	==================================
            --- ENUMS ---
	==================================
*/

/*

         commc_socket_type_t
	       ---
	       enumeration of socket types supported
	       by the wrapper library.

*/

typedef enum {
    COMMC_SOCKET_TYPE_TCP = 0,
    COMMC_SOCKET_TYPE_UDP
} commc_socket_type_t;

/*

         commc_socket_family_t
	       ---
	       enumeration of socket address families
	       for protocol selection.

*/

typedef enum {
    COMMC_SOCKET_FAMILY_IPV4 = 0,
    COMMC_SOCKET_FAMILY_IPV6,
    COMMC_SOCKET_FAMILY_UNSPEC
} commc_socket_family_t;

/*

         commc_socket_state_t
	       ---
	       enumeration of socket connection states
	       for status tracking.

*/

typedef enum {
    COMMC_SOCKET_STATE_CLOSED = 0,
    COMMC_SOCKET_STATE_LISTENING,
    COMMC_SOCKET_STATE_CONNECTING,
    COMMC_SOCKET_STATE_CONNECTED,
    COMMC_SOCKET_STATE_ERROR
} commc_socket_state_t;

/* 
	==================================
           --- STRUCTURES ---
	==================================
*/

/*

         commc_socket_address_t
	       ---
	       structure representing a network address
	       with hostname and port information.

*/

typedef struct {
    char hostname[COMMC_SOCKET_MAX_HOSTNAME_LENGTH];
    char service[COMMC_SOCKET_MAX_SERVICE_LENGTH];
    int  port;
    commc_socket_family_t family;
} commc_socket_address_t;

/*

         commc_socket_options_t
	       ---
	       structure containing socket configuration
	       options and behavioral settings.

*/

typedef struct {
    int  reuse_address;        /* SO_REUSEADDR option */
    int  keep_alive;          /* SO_KEEPALIVE option */
    int  no_delay;            /* TCP_NODELAY option */
    int  broadcast;           /* SO_BROADCAST option */
    int  receive_timeout;     /* Receive timeout in seconds */
    int  send_timeout;        /* Send timeout in seconds */
    int  receive_buffer_size; /* SO_RCVBUF size */
    int  send_buffer_size;    /* SO_SNDBUF size */
} commc_socket_options_t;

/*

         commc_socket_t
	       ---
	       main socket structure containing platform-specific
	       handle and associated metadata.

*/

typedef struct {
    int                        handle;       /* Platform socket handle */
    commc_socket_type_t        type;         /* TCP or UDP */
    commc_socket_family_t      family;       /* Address family */
    commc_socket_state_t       state;        /* Connection state */
    commc_socket_address_t     local_addr;   /* Local address info */
    commc_socket_address_t     remote_addr;  /* Remote address info */
    commc_socket_options_t     options;      /* Socket options */
    int                        last_error;   /* Last error code */
} commc_socket_t;

/*

         commc_socket_server_t
	       ---
	       structure for managing server socket
	       operations and client connections.

*/

typedef struct {
    commc_socket_t*  listening_socket;  /* Main listening socket */
    commc_socket_t** client_sockets;    /* Array of client sockets */
    int              max_clients;       /* Maximum client connections */
    int              current_clients;   /* Current client count */
    int              backlog;           /* Listen backlog size */
} commc_socket_server_t;

/* 
	==================================
        --- INITIALIZATION ---
	==================================
*/

/*

         commc_socket_init()
	       ---
	       initializes the socket subsystem. must be called
	       before any other socket operations on Windows.

*/

commc_error_t commc_socket_init(void);

/*

         commc_socket_cleanup()
	       ---
	       cleans up the socket subsystem resources.
	       should be called at program exit.

*/

void commc_socket_cleanup(void);

/* 
	==================================
      --- SOCKET MANAGEMENT ---
	==================================
*/

/*

         commc_socket_create()
	       ---
	       creates a new socket with specified type
	       and address family.

*/

commc_error_t commc_socket_create(commc_socket_t**        socket,
                                  commc_socket_type_t     type,
                                  commc_socket_family_t   family);

/*

         commc_socket_destroy()
	       ---
	       closes socket and frees associated resources.

*/

void commc_socket_destroy(commc_socket_t* socket);

/*

         commc_socket_set_options()
	       ---
	       applies socket options to configure behavior.

*/

commc_error_t commc_socket_set_options(commc_socket_t*              socket,
                                       const commc_socket_options_t* options);

/*

         commc_socket_get_options()
	       ---
	       retrieves current socket option settings.

*/

commc_error_t commc_socket_get_options(const commc_socket_t*    socket,
                                       commc_socket_options_t*  options);

/* 
	==================================
        --- CLIENT OPERATIONS ---
	==================================
*/

/*

         commc_socket_connect()
	       ---
	       establishes connection to remote host
	       with specified timeout.

*/

commc_error_t commc_socket_connect(commc_socket_t*                socket,
                                   const commc_socket_address_t*  address,
                                   int                            timeout_seconds);

/*

         commc_socket_connect_hostname()
	       ---
	       convenience function to connect using
	       hostname and port strings.

*/

commc_error_t commc_socket_connect_hostname(commc_socket_t* socket,
                                            const char*     hostname,
                                            const char*     service,
                                            int             timeout_seconds);

/*

         commc_socket_disconnect()
	       ---
	       gracefully closes connection and resets
	       socket state to closed.

*/

commc_error_t commc_socket_disconnect(commc_socket_t* socket);

/* 
	==================================
        --- SERVER OPERATIONS ---
	==================================
*/

/*

         commc_socket_bind()
	       ---
	       binds socket to local address and port
	       for server operations.

*/

commc_error_t commc_socket_bind(commc_socket_t*               socket,
                                const commc_socket_address_t* address);

/*

         commc_socket_listen()
	       ---
	       puts socket in listening state for
	       incoming connections.

*/

commc_error_t commc_socket_listen(commc_socket_t* socket,
                                  int             backlog);

/*

         commc_socket_accept()
	       ---
	       accepts pending client connection and
	       creates new socket for communication.

*/

commc_error_t commc_socket_accept(commc_socket_t*  server_socket,
                                  commc_socket_t** client_socket,
                                  int              timeout_seconds);

/* 
	==================================
         --- DATA TRANSFER ---
	==================================
*/

/*

         commc_socket_send()
	       ---
	       sends data through connected socket with
	       timeout and partial send handling.

*/

commc_error_t commc_socket_send(commc_socket_t* socket,
                                const void*     data,
                                size_t          data_length,
                                size_t*         bytes_sent);

/*

         commc_socket_receive()
	       ---
	       receives data from connected socket with
	       timeout and buffer management.

*/

commc_error_t commc_socket_receive(commc_socket_t* socket,
                                   void*           buffer,
                                   size_t          buffer_size,
                                   size_t*         bytes_received);

/*

         commc_socket_send_all()
	       ---
	       ensures all data is sent by handling
	       partial sends automatically.

*/

commc_error_t commc_socket_send_all(commc_socket_t* socket,
                                    const void*     data,
                                    size_t          data_length);

/*

         commc_socket_receive_all()
	       ---
	       receives exact amount of data, blocking
	       until complete or timeout occurs.

*/

commc_error_t commc_socket_receive_all(commc_socket_t* socket,
                                       void*           buffer,
                                       size_t          expected_length);

/* 
	==================================
         --- UDP OPERATIONS ---
	==================================
*/

/*

         commc_socket_sendto()
	       ---
	       sends UDP datagram to specified address
	       without establishing connection.

*/

commc_error_t commc_socket_sendto(commc_socket_t*               socket,
                                  const void*                   data,
                                  size_t                        data_length,
                                  const commc_socket_address_t* dest_address,
                                  size_t*                       bytes_sent);

/*

         commc_socket_receivefrom()
	       ---
	       receives UDP datagram and captures
	       sender's address information.

*/

commc_error_t commc_socket_receivefrom(commc_socket_t*         socket,
                                       void*                   buffer,
                                       size_t                  buffer_size,
                                       commc_socket_address_t* src_address,
                                       size_t*                 bytes_received);

/* 
	==================================
        --- ADDRESS UTILITIES ---
	==================================
*/

/*

         commc_socket_address_create()
	       ---
	       creates socket address structure from
	       hostname and port components.

*/

commc_error_t commc_socket_address_create(commc_socket_address_t*   address,
                                          const char*               hostname,
                                          const char*               service,
                                          commc_socket_family_t     family);

/*

         commc_socket_address_resolve()
	       ---
	       resolves hostname to IP address using
	       system DNS resolution.

*/

commc_error_t commc_socket_address_resolve(const commc_socket_address_t* address,
                                           char*                         resolved_ip,
                                           size_t                        ip_buffer_size);

/*

         commc_socket_get_local_address()
	       ---
	       retrieves local address information
	       for connected socket.

*/

commc_error_t commc_socket_get_local_address(const commc_socket_t*   socket,
                                             commc_socket_address_t* address);

/*

         commc_socket_get_remote_address()
	       ---
	       retrieves remote address information
	       for connected socket.

*/

commc_error_t commc_socket_get_remote_address(const commc_socket_t*   socket,
                                              commc_socket_address_t* address);

/* 
	==================================
        --- SERVER MANAGEMENT ---
	==================================
*/

/*

         commc_socket_server_create()
	       ---
	       creates server management structure
	       for handling multiple clients.

*/

commc_error_t commc_socket_server_create(commc_socket_server_t**     server,
                                         const commc_socket_address_t* bind_address,
                                         int                           max_clients);

/*

         commc_socket_server_destroy()
	       ---
	       destroys server and disconnects all
	       active client connections.

*/

void commc_socket_server_destroy(commc_socket_server_t* server);

/*

         commc_socket_server_start()
	       ---
	       starts server listening for incoming
	       client connections.

*/

commc_error_t commc_socket_server_start(commc_socket_server_t* server);

/*

         commc_socket_server_accept_client()
	       ---
	       accepts new client connection and adds
	       to server's client list.

*/

commc_error_t commc_socket_server_accept_client(commc_socket_server_t* server,
                                                int                    timeout_seconds);

/* 
	==================================
        --- STATUS AND UTILITIES ---
	==================================
*/

/*

         commc_socket_get_error()
	       ---
	       retrieves last error code for socket
	       operations and debugging.

*/

int commc_socket_get_error(const commc_socket_t* socket);

/*

         commc_socket_is_connected()
	       ---
	       checks if socket is in connected state
	       and ready for data transfer.

*/

int commc_socket_is_connected(const commc_socket_t* socket);

/*

         commc_socket_set_blocking()
	       ---
	       configures socket blocking mode for
	       asynchronous operations.

*/

commc_error_t commc_socket_set_blocking(commc_socket_t* socket,
                                        int             blocking);

/*

         commc_socket_wait_readable()
	       ---
	       waits for socket to become readable
	       with specified timeout.

*/

commc_error_t commc_socket_wait_readable(const commc_socket_t* socket,
                                         int                   timeout_seconds);

/*

         commc_socket_wait_writable()
	       ---
	       waits for socket to become writable
	       with specified timeout.

*/

commc_error_t commc_socket_wait_writable(const commc_socket_t* socket,
                                         int                   timeout_seconds);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_SOCKET_H */

/* 
	==================================
             --- EOF ---
	==================================
*/