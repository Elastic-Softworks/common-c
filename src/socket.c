/* 	
   ===================================
   S O C K E T . C
   CROSS-PLATFORM SOCKET WRAPPER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    cross-platform Berkeley sockets implementation
	    providing unified interface for TCP/UDP networking.
	    handles platform differences while maintaining
	    educational clarity and robust error management.
	       
	    implements comprehensive client/server operations
	    with proper timeout handling, address resolution,
	    and connection management for reliable networking.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif
#define close closesocket
#ifndef EINPROGRESS
#define EINPROGRESS WSAEWOULDBLOCK
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#ifndef EAGAIN
#define EAGAIN      WSAEWOULDBLOCK
#endif
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "commc/socket.h"
#include "commc/error.h"

/* 
	==================================
           --- CONSTANTS ---
	==================================
*/

static int socket_initialized = 0;

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         get_socket_error()
	       ---
	       retrieves the last socket error code
	       in a cross-platform manner.

*/

static int get_socket_error(void) {

#ifdef _WIN32

    return WSAGetLastError();
    
#else

    return errno;
    
#endif

}

/*

         set_socket_error()
	       ---
	       sets socket error in the socket structure
	       for debugging and error reporting.

*/

static void set_socket_error(commc_socket_t* socket, int error_code) {

    if (socket) {
    
        socket->last_error = error_code;
        
        if (error_code != 0) {
        
            socket->state = COMMC_SOCKET_STATE_ERROR;
            
        }
        
    }
    
}

/*

         socket_type_to_int()
	       ---
	       converts socket type enumeration to
	       system socket type constant.

*/

static int socket_type_to_int(commc_socket_type_t type) {

    switch (type) {
    
        case COMMC_SOCKET_TYPE_TCP:
            return SOCK_STREAM;
            
        case COMMC_SOCKET_TYPE_UDP:
            return SOCK_DGRAM;
            
        default:
            return SOCK_STREAM;
            
    }
    
}

/*

         family_to_int()
	       ---
	       converts address family enumeration to
	       system address family constant.

*/

static int family_to_int(commc_socket_family_t family) {

    switch (family) {
    
        case COMMC_SOCKET_FAMILY_IPV4:
            return AF_INET;
            
        case COMMC_SOCKET_FAMILY_IPV6:
            return AF_INET6;
            
        case COMMC_SOCKET_FAMILY_UNSPEC:
            return AF_UNSPEC;
            
        default:
            return AF_INET;
            
    }
    
}

/*

         wait_for_socket()
	       ---
	       waits for socket to become ready for
	       reading or writing with timeout.

*/

static commc_error_t wait_for_socket(int socket_fd,
                                     int for_writing,
                                     int timeout_seconds) {

    fd_set         fds;
    struct timeval timeout;
    int            result;
    
    if (socket_fd == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);
    
    timeout.tv_sec  = timeout_seconds;
    timeout.tv_usec = 0;
    
    if (for_writing) {
    
        result = select(socket_fd + 1, NULL, &fds, NULL, &timeout);
        
    } else {
    
        result = select(socket_fd + 1, &fds, NULL, NULL, &timeout);
        
    }
    
    if (result == COMMC_SOCKET_ERROR) {
    
        return COMMC_SYSTEM_ERROR;
        
    }
    
    if (result == 0) {
    
        return COMMC_ERROR_TIMEOUT;
        
    }
    
    return COMMC_SUCCESS;
    
}

/* 
	==================================
        --- INITIALIZATION ---
	==================================
*/

/*

         commc_socket_init()
	       ---
	       initializes the socket subsystem.

*/

commc_error_t commc_socket_init(void) {

#ifdef _WIN32

    WSADATA wsa_data;
    int     result;
    
    if (socket_initialized) {
    
        return COMMC_SUCCESS;
        
    }
    
    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    
    if (result != 0) {
    
        return COMMC_SYSTEM_ERROR;
        
    }
    
#endif

    socket_initialized = 1;
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_cleanup()
	       ---
	       cleans up the socket subsystem resources.

*/

void commc_socket_cleanup(void) {

    if (!socket_initialized) {
    
        return;
        
    }
    
#ifdef _WIN32

    WSACleanup();
    
#endif

    socket_initialized = 0;
    
}

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
                                  commc_socket_family_t   family) {

    commc_socket_t* new_socket;
    int             sock_fd;
    int             sock_type;
    int             sock_family;
    
    if (!socket) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Initialize socket system if needed */
    
    if (!socket_initialized) {
    
        commc_error_t init_result = commc_socket_init();
        
        if (init_result != COMMC_SUCCESS) {
        
            return init_result;
            
        }
        
    }
    
    /* Allocate socket structure */
    
    new_socket = malloc(sizeof(commc_socket_t));
    
    if (!new_socket) {
    
        return COMMC_MEMORY_ERROR;
        
    }
    
    /* Initialize socket structure */
    
    memset(new_socket, 0, sizeof(commc_socket_t));
    
    new_socket->handle = COMMC_INVALID_SOCKET;
    new_socket->type   = type;
    new_socket->family = family;
    new_socket->state  = COMMC_SOCKET_STATE_CLOSED;
    
    /* Set default options */
    
    new_socket->options.reuse_address      = 0;
    new_socket->options.keep_alive         = 0;
    new_socket->options.no_delay           = 0;
    new_socket->options.broadcast          = 0;
    new_socket->options.receive_timeout    = COMMC_SOCKET_DEFAULT_TIMEOUT;
    new_socket->options.send_timeout       = COMMC_SOCKET_DEFAULT_TIMEOUT;
    new_socket->options.receive_buffer_size = COMMC_SOCKET_BUFFER_SIZE;
    new_socket->options.send_buffer_size    = COMMC_SOCKET_BUFFER_SIZE;
    
    /* Create system socket */
    
    sock_type   = socket_type_to_int(type);
    sock_family = family_to_int(family);
    
#ifdef _WIN32
    sock_fd = WSASocket(sock_family, sock_type, 0, NULL, 0, 0);
#else
    sock_fd = socket(sock_family, sock_type, 0);
#endif
    
    if (sock_fd == COMMC_INVALID_SOCKET) {
    
        set_socket_error(new_socket, get_socket_error());
        free(new_socket);
        return COMMC_SYSTEM_ERROR;
        
    }
    
    new_socket->handle = sock_fd;
    *socket = new_socket;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_destroy()
	       ---
	       closes socket and frees associated resources.

*/

void commc_socket_destroy(commc_socket_t* socket) {

    if (!socket) {
    
        return;
        
    }
    
    if (socket->handle != COMMC_INVALID_SOCKET) {
    
        close(socket->handle);
        socket->handle = COMMC_INVALID_SOCKET;
        
    }
    
    socket->state = COMMC_SOCKET_STATE_CLOSED;
    free(socket);
    
}

/*

         commc_socket_set_options()
	       ---
	       applies socket options to configure behavior.

*/

commc_error_t commc_socket_set_options(commc_socket_t*              socket,
                                       const commc_socket_options_t* options) {

    int result;
    int option_value;
    
    if (!socket || !options || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Copy options to socket */
    
    socket->options = *options;
    
    /* Set SO_REUSEADDR */
    
    if (options->reuse_address) {
    
        option_value = 1;
        result = setsockopt(socket->handle, SOL_SOCKET, SO_REUSEADDR,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    /* Set SO_KEEPALIVE */
    
    if (options->keep_alive && socket->type == COMMC_SOCKET_TYPE_TCP) {
    
        option_value = 1;
        result = setsockopt(socket->handle, SOL_SOCKET, SO_KEEPALIVE,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    /* Set TCP_NODELAY */
    
    if (options->no_delay && socket->type == COMMC_SOCKET_TYPE_TCP) {
    
        option_value = 1;
        result = setsockopt(socket->handle, IPPROTO_TCP, TCP_NODELAY,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    /* Set SO_BROADCAST */
    
    if (options->broadcast && socket->type == COMMC_SOCKET_TYPE_UDP) {
    
        option_value = 1;
        result = setsockopt(socket->handle, SOL_SOCKET, SO_BROADCAST,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    /* Set receive buffer size */
    
    if (options->receive_buffer_size > 0) {
    
        option_value = options->receive_buffer_size;
        result = setsockopt(socket->handle, SOL_SOCKET, SO_RCVBUF,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    /* Set send buffer size */
    
    if (options->send_buffer_size > 0) {
    
        option_value = options->send_buffer_size;
        result = setsockopt(socket->handle, SOL_SOCKET, SO_SNDBUF,
                           (const char*)&option_value, sizeof(option_value));
                           
        if (result == COMMC_SOCKET_ERROR) {
        
            set_socket_error(socket, get_socket_error());
            return COMMC_SYSTEM_ERROR;
            
        }
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_get_options()
	       ---
	       retrieves current socket option settings.

*/

commc_error_t commc_socket_get_options(const commc_socket_t*    socket,
                                       commc_socket_options_t*  options) {

    if (!socket || !options) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    *options = socket->options;
    
    return COMMC_SUCCESS;
    
}

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
                                          commc_socket_family_t     family) {

    if (!address || !hostname || !service) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    memset(address, 0, sizeof(commc_socket_address_t));
    
    /* Copy hostname and service */
    
    if (strlen(hostname) >= COMMC_SOCKET_MAX_HOSTNAME_LENGTH) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    if (strlen(service) >= COMMC_SOCKET_MAX_SERVICE_LENGTH) {
    
        return COMMC_ERROR_BUFFER_TOO_SMALL;
        
    }
    
    strcpy(address->hostname, hostname);
    strcpy(address->service, service);
    address->family = family;
    
    /* Convert service to port number if it's numeric */
    
    address->port = atoi(service);
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_address_resolve()
	       ---
	       resolves hostname to IP address using
	       system DNS resolution.

*/

commc_error_t commc_socket_address_resolve(const commc_socket_address_t* address,
                                           char*                         resolved_ip,
                                           size_t                        ip_buffer_size) {

    struct addrinfo  hints;
    struct addrinfo* result;
    struct addrinfo* addr_ptr;
    int              getaddr_result;
    
    if (!address || !resolved_ip || ip_buffer_size == 0) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = family_to_int(address->family);
    hints.ai_socktype = SOCK_STREAM;
    
    getaddr_result = getaddrinfo(address->hostname, address->service, &hints, &result);
    
    if (getaddr_result != 0) {
    
        return COMMC_SYSTEM_ERROR;
        
    }
    
    for (addr_ptr = result; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {
    
        if (addr_ptr->ai_family == AF_INET) {
        
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr_ptr->ai_addr;
            
            if (inet_ntop(AF_INET, &ipv4->sin_addr, resolved_ip, (socklen_t)ip_buffer_size)) {
            
                freeaddrinfo(result);
                return COMMC_SUCCESS;
                
            }
            
        } else if (addr_ptr->ai_family == AF_INET6) {
        
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr_ptr->ai_addr;
            
            if (inet_ntop(AF_INET6, &ipv6->sin6_addr, resolved_ip, (socklen_t)ip_buffer_size)) {
            
                freeaddrinfo(result);
                return COMMC_SUCCESS;
                
            }
            
        }
        
    }
    
    freeaddrinfo(result);
    return COMMC_SYSTEM_ERROR;
    
}

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
                                   int                            timeout_seconds) {

    struct addrinfo  hints;
    struct addrinfo* result;
    struct addrinfo* addr_ptr;
    int              connect_result;
    int              getaddr_result;
    commc_error_t    wait_result;
    
    if (!socket || !address || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    socket->state = COMMC_SOCKET_STATE_CONNECTING;
    
    /* Resolve address */
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = family_to_int(address->family);
    hints.ai_socktype = socket_type_to_int(socket->type);
    
    getaddr_result = getaddrinfo(address->hostname, address->service, &hints, &result);
    
    if (getaddr_result != 0) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Try connecting to resolved addresses */
    
    for (addr_ptr = result; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {
    
        connect_result = connect(socket->handle, addr_ptr->ai_addr, (socklen_t)addr_ptr->ai_addrlen);
        
        if (connect_result == 0) {
        
            /* Connection successful */
            
            socket->state = COMMC_SOCKET_STATE_CONNECTED;
            socket->remote_addr = *address;
            
            freeaddrinfo(result);
            return COMMC_SUCCESS;
            
        }
        
        /* Handle non-blocking connect */
        
        if (get_socket_error() == EINPROGRESS || get_socket_error() == EWOULDBLOCK) {
        
            wait_result = wait_for_socket(socket->handle, 1, timeout_seconds);
            
            if (wait_result == COMMC_SUCCESS) {
            
                socket->state = COMMC_SOCKET_STATE_CONNECTED;
                socket->remote_addr = *address;
                
                freeaddrinfo(result);
                return COMMC_SUCCESS;
                
            }
            
        }
        
    }
    
    freeaddrinfo(result);
    set_socket_error(socket, get_socket_error());
    return COMMC_SYSTEM_ERROR;
    
}

/*

         commc_socket_connect_hostname()
	       ---
	       convenience function to connect using
	       hostname and port strings.

*/

commc_error_t commc_socket_connect_hostname(commc_socket_t* socket,
                                            const char*     hostname,
                                            const char*     service,
                                            int             timeout_seconds) {

    commc_socket_address_t address;
    commc_error_t          result;
    
    result = commc_socket_address_create(&address, hostname, service, socket->family);
    
    if (result != COMMC_SUCCESS) {
    
        return result;
        
    }
    
    return commc_socket_connect(socket, &address, timeout_seconds);
    
}

/*

         commc_socket_disconnect()
	       ---
	       gracefully closes connection and resets
	       socket state to closed.

*/

commc_error_t commc_socket_disconnect(commc_socket_t* socket) {

    if (!socket || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (socket->type == COMMC_SOCKET_TYPE_TCP) {
    
        shutdown(socket->handle, 2); /* SHUT_RDWR equivalent */
        
    }
    
    close(socket->handle);
    socket->handle = COMMC_INVALID_SOCKET;
    socket->state  = COMMC_SOCKET_STATE_CLOSED;
    
    memset(&socket->remote_addr, 0, sizeof(socket->remote_addr));
    
    return COMMC_SUCCESS;
    
}

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
                                const commc_socket_address_t* address) {

    struct addrinfo  hints;
    struct addrinfo* result;
    struct addrinfo* addr_ptr;
    int              bind_result;
    int              getaddr_result;
    
    if (!socket || !address || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Resolve address */
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = family_to_int(address->family);
    hints.ai_socktype = socket_type_to_int(socket->type);
    hints.ai_flags    = AI_PASSIVE;
    
    getaddr_result = getaddrinfo(address->hostname, address->service, &hints, &result);
    
    if (getaddr_result != 0) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Try binding to resolved addresses */
    
    for (addr_ptr = result; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {
    
        bind_result = bind(socket->handle, addr_ptr->ai_addr, (socklen_t)addr_ptr->ai_addrlen);
        
        if (bind_result == 0) {
        
            socket->local_addr = *address;
            
            freeaddrinfo(result);
            return COMMC_SUCCESS;
            
        }
        
    }
    
    freeaddrinfo(result);
    set_socket_error(socket, get_socket_error());
    return COMMC_SYSTEM_ERROR;
    
}

/*

         commc_socket_listen()
	       ---
	       puts socket in listening state for
	       incoming connections.

*/

commc_error_t commc_socket_listen(commc_socket_t* socket,
                                  int             backlog) {

    int listen_result;
    
    if (!socket || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (socket->type != COMMC_SOCKET_TYPE_TCP) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    if (backlog <= 0) {
    
        backlog = COMMC_SOCKET_MAX_BACKLOG;
        
    }
    
    listen_result = listen(socket->handle, backlog);
    
    if (listen_result == COMMC_SOCKET_ERROR) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
    socket->state = COMMC_SOCKET_STATE_LISTENING;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_accept()
	       ---
	       accepts pending client connection and
	       creates new socket for communication.

*/

commc_error_t commc_socket_accept(commc_socket_t*  server_socket,
                                  commc_socket_t** client_socket,
                                  int              timeout_seconds) {

    commc_socket_t*    new_client;
    struct sockaddr_in client_addr;
    socklen_t          addr_len;
    int                accept_fd;
    commc_error_t      wait_result;
    commc_error_t      create_result;
    
    if (!server_socket || !client_socket || 
        server_socket->handle == COMMC_INVALID_SOCKET ||
        server_socket->state != COMMC_SOCKET_STATE_LISTENING) {
        
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    /* Wait for incoming connection */
    
    if (timeout_seconds > 0) {
    
        wait_result = wait_for_socket(server_socket->handle, 0, timeout_seconds);
        
        if (wait_result != COMMC_SUCCESS) {
        
            return wait_result;
            
        }
        
    }
    
    /* Accept connection */
    
    addr_len = sizeof(client_addr);
    accept_fd = accept(server_socket->handle, (struct sockaddr*)&client_addr, &addr_len);
    
    if (accept_fd == COMMC_INVALID_SOCKET) {
    
        set_socket_error(server_socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
    /* Create client socket structure */
    
    create_result = commc_socket_create(&new_client, server_socket->type, server_socket->family);
    
    if (create_result != COMMC_SUCCESS) {
    
        close(accept_fd);
        return create_result;
        
    }
    
    /* Close the socket created by commc_socket_create and use accepted socket */
    
    close(new_client->handle);
    new_client->handle = accept_fd;
    new_client->state  = COMMC_SOCKET_STATE_CONNECTED;
    
    /* Set client address information */
    
    inet_ntop(AF_INET, &client_addr.sin_addr, 
              new_client->remote_addr.hostname, 
              COMMC_SOCKET_MAX_HOSTNAME_LENGTH);
              
    new_client->remote_addr.port = ntohs(client_addr.sin_port);
    
    *client_socket = new_client;
    
    return COMMC_SUCCESS;
    
}

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
                                size_t*         bytes_sent) {

    int send_result;
    
    if (!socket || !data || !bytes_sent || 
        socket->handle == COMMC_INVALID_SOCKET ||
        socket->state != COMMC_SOCKET_STATE_CONNECTED) {
        
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    send_result = send(socket->handle, (const char*)data, (int)data_length, 0);
    
    if (send_result == COMMC_SOCKET_ERROR) {
    
        int error_code = get_socket_error();
        
        if (error_code == EWOULDBLOCK || error_code == EAGAIN) {
        
            *bytes_sent = 0;
            return COMMC_ERROR_WOULD_BLOCK;
            
        }
        
        set_socket_error(socket, error_code);
        return COMMC_SYSTEM_ERROR;
        
    }
    
    *bytes_sent = (size_t)send_result;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_receive()
	       ---
	       receives data from connected socket with
	       timeout and buffer management.

*/

commc_error_t commc_socket_receive(commc_socket_t* socket,
                                   void*           buffer,
                                   size_t          buffer_size,
                                   size_t*         bytes_received) {

    int recv_result;
    
    if (!socket || !buffer || !bytes_received || 
        socket->handle == COMMC_INVALID_SOCKET ||
        socket->state != COMMC_SOCKET_STATE_CONNECTED) {
        
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    recv_result = recv(socket->handle, (char*)buffer, (int)buffer_size, 0);
    
    if (recv_result == COMMC_SOCKET_ERROR) {
    
        int error_code = get_socket_error();
        
        if (error_code == EWOULDBLOCK || error_code == EAGAIN) {
        
            *bytes_received = 0;
            return COMMC_ERROR_WOULD_BLOCK;
            
        }
        
        set_socket_error(socket, error_code);
        return COMMC_SYSTEM_ERROR;
        
    }
    
    if (recv_result == 0) {
    
        /* Connection closed */
        
        socket->state = COMMC_SOCKET_STATE_CLOSED;
        *bytes_received = 0;
        return COMMC_ERROR_CONNECTION_CLOSED;
        
    }
    
    *bytes_received = (size_t)recv_result;
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_send_all()
	       ---
	       ensures all data is sent by handling
	       partial sends automatically.

*/

commc_error_t commc_socket_send_all(commc_socket_t* socket,
                                    const void*     data,
                                    size_t          data_length) {

    size_t        total_sent = 0;
    size_t        bytes_sent;
    commc_error_t result;
    
    while (total_sent < data_length) {
    
        result = commc_socket_send(socket, 
                                  (const char*)data + total_sent,
                                  data_length - total_sent,
                                  &bytes_sent);
                                  
        if (result != COMMC_SUCCESS) {
        
            if (result == COMMC_ERROR_WOULD_BLOCK) {
            
                /* Wait for socket to become writable */
                
                result = wait_for_socket(socket->handle, 1, socket->options.send_timeout);
                
                if (result != COMMC_SUCCESS) {
                
                    return result;
                    
                }
                
                continue;
                
            }
            
            return result;
            
        }
        
        total_sent += bytes_sent;
        
    }
    
    return COMMC_SUCCESS;
    
}

/*

         commc_socket_receive_all()
	       ---
	       receives exact amount of data, blocking
	       until complete or timeout occurs.

*/

commc_error_t commc_socket_receive_all(commc_socket_t* socket,
                                       void*           buffer,
                                       size_t          expected_length) {

    size_t        total_received = 0;
    size_t        bytes_received;
    commc_error_t result;
    
    while (total_received < expected_length) {
    
        result = commc_socket_receive(socket,
                                     (char*)buffer + total_received,
                                     expected_length - total_received,
                                     &bytes_received);
                                     
        if (result != COMMC_SUCCESS) {
        
            if (result == COMMC_ERROR_WOULD_BLOCK) {
            
                /* Wait for socket to become readable */
                
                result = wait_for_socket(socket->handle, 0, socket->options.receive_timeout);
                
                if (result != COMMC_SUCCESS) {
                
                    return result;
                    
                }
                
                continue;
                
            }
            
            return result;
            
        }
        
        total_received += bytes_received;
        
    }
    
    return COMMC_SUCCESS;
    
}

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

int commc_socket_get_error(const commc_socket_t* socket) {

    if (!socket) {
    
        return 0;
        
    }
    
    return socket->last_error;
    
}

/*

         commc_socket_is_connected()
	       ---
	       checks if socket is in connected state
	       and ready for data transfer.

*/

int commc_socket_is_connected(const commc_socket_t* socket) {

    if (!socket) {
    
        return 0;
        
    }
    
    return (socket->state == COMMC_SOCKET_STATE_CONNECTED) ? 1 : 0;
    
}

/*

         commc_socket_set_blocking()
	       ---
	       configures socket blocking mode for
	       asynchronous operations.

*/

commc_error_t commc_socket_set_blocking(commc_socket_t* socket,
                                        int             blocking) {

#ifdef _WIN32
    u_long mode;
#else
    int flags;
#endif
    
    if (!socket || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
#ifdef _WIN32

    mode = blocking ? 0 : 1;
    
    if (ioctlsocket(socket->handle, FIONBIO, &mode) == COMMC_SOCKET_ERROR) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
#else

    flags = fcntl(socket->handle, F_GETFL, 0);
    
    if (flags == -1) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
    if (blocking) {
    
        flags &= ~O_NONBLOCK;
        
    } else {
    
        flags |= O_NONBLOCK;
        
    }
    
    if (fcntl(socket->handle, F_SETFL, flags) == -1) {
    
        set_socket_error(socket, get_socket_error());
        return COMMC_SYSTEM_ERROR;
        
    }
    
#endif

    return COMMC_SUCCESS;
    
}

/*

         commc_socket_wait_readable()
	       ---
	       waits for socket to become readable
	       with specified timeout.

*/

commc_error_t commc_socket_wait_readable(const commc_socket_t* socket,
                                         int                   timeout_seconds) {

    if (!socket || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    return wait_for_socket(socket->handle, 0, timeout_seconds);
    
}

/*

         commc_socket_wait_writable()
	       ---
	       waits for socket to become writable
	       with specified timeout.

*/

commc_error_t commc_socket_wait_writable(const commc_socket_t* socket,
                                         int                   timeout_seconds) {

    if (!socket || socket->handle == COMMC_INVALID_SOCKET) {
    
        return COMMC_ERROR_INVALID_ARGUMENT;
        
    }
    
    return wait_for_socket(socket->handle, 1, timeout_seconds);
    
}

/* 
	==================================
             --- EOF ---
	==================================
*/