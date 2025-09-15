/*
   ===================================
   C O M M O N - C
   NETWORKING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- NET MODULE ---

    implementation of cross-platform networking
    abstractions.
    see include/commc/net.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/


#include <stdio.h>         /* for snprintf */
#include <string.h>        /* for memset */

#include "commc/net.h"
#include "commc/error.h"   /* for error handling */

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>      /* for getaddrinfo */

#pragma comment(lib, "ws2_32.lib")

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>         /* for getaddrinfo */
#include <unistd.h>        /* for close */

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#endif

/*
	==================================
             --- STATIC GLOBALS ---
	==================================
*/

#ifdef _WIN32

static WSADATA   wsa_data;
static int       net_initialized = 0;

#endif

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_net_init()
	       ---
	       initializes the networking subsystem.
	       on windows, this calls WSAStartup.

*/

int commc_net_init(void) {

#ifdef _WIN32

  if  (net_initialized) {

    return 1; /* already initialized */

  }
  if  (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {

    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return 0;

  }

  net_initialized = 1;

#endif

  return 1;

}

/*

         commc_net_shutdown()
	       ---
	       shuts down the networking subsystem.
	       on windows, this calls WSACleanup.

*/

void commc_net_shutdown(void) {

#ifdef _WIN32

  if  (net_initialized) {

    WSACleanup();
    net_initialized = 0;

  }

#endif

}

/*

         commc_net_socket_create()
	       ---
	       creates a new socket.

*/

commc_socket_t commc_net_socket_create(commc_net_type_t type) {

  commc_socket_t sock = INVALID_SOCKET;

  int  protocol;
  int  sock_type;

  if  (type == COMMC_NET_TCP) {

    sock_type = SOCK_STREAM;
    protocol  = IPPROTO_TCP;

  } else if  (type == COMMC_NET_UDP) {

    sock_type = SOCK_DGRAM;
    protocol  = IPPROTO_UDP;

  } else {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return INVALID_SOCKET;

  }

  sock = (commc_socket_t)socket(AF_INET, sock_type, protocol);

  if  (sock == INVALID_SOCKET) {

    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);

  }

  return sock;

}

/*

         commc_net_socket_close()
	       ---
	       closes an open socket.

*/

void commc_net_socket_close(commc_socket_t sock) {

  if  (sock != INVALID_SOCKET) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
  }

}

/*

         commc_net_bind()
	       ---
	       binds a socket to a specific port.

*/

int commc_net_bind(commc_socket_t sock, unsigned short port) {

  struct sockaddr_in addr;
  
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); /* bind to all interfaces */
  addr.sin_port        = htons(port);

  if  (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return 0;
  }

  return 1;

}

/*

         commc_net_listen()
	       ---
	       sets a TCP socket to listen for connections.

*/

int commc_net_listen(commc_socket_t sock, int backlog) {

  if  (listen(sock, backlog) == SOCKET_ERROR) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return 0;
  }

  return 1;

}

/*

         commc_net_accept()
	       ---
	       accepts an incoming TCP connection.

*/

commc_socket_t commc_net_accept(commc_socket_t listen_sock) {

  commc_socket_t client_sock;
  
  client_sock = (commc_socket_t)accept(listen_sock, NULL, NULL);

  if  (client_sock == INVALID_SOCKET) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
  }

  return client_sock;

}

/*

         commc_net_connect()
	       ---
	       connects a TCP socket to a remote host.

*/

int commc_net_connect(commc_socket_t sock, const char* host, unsigned short port) {

  struct addrinfo  hints;
  struct addrinfo* res;
  struct addrinfo* p;
  char             port_str[6]; /* max 5 digits + null terminator */
  int              status;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  sprintf(port_str, "%u", port);

  status = getaddrinfo(host, port_str, &hints, &res);

  if  (status != 0) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return 0;
  }

  for  (p = res; p != NULL; p = p->ai_next) {
    if  (connect(sock, p->ai_addr, (int)p->ai_addrlen) == 0) {
      freeaddrinfo(res);
      return 1; /* success */
    }
  }

  freeaddrinfo(res);
  commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
  return 0;

}

/*

         commc_net_send()
	       ---
	       sends data over a TCP socket.

*/

int commc_net_send(commc_socket_t sock, const void* buffer, size_t len) {

  int bytes_sent;
  
  bytes_sent = send(sock, (const char*)buffer, (int)len, 0);

  if  (bytes_sent == SOCKET_ERROR) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return -1;
  }

  return bytes_sent;

}

/*

         commc_net_recv()
	       ---
	       receives data from a TCP socket.

*/

int commc_net_recv(commc_socket_t sock, void* buffer, size_t len) {

  int bytes_recv;
  
  bytes_recv = recv(sock, (char*)buffer, (int)len, 0);

  if  (bytes_recv == SOCKET_ERROR) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return -1;
  }

  return bytes_recv;

}

/*

         commc_net_sendto()
	       ---
	       sends UDP data to a specific host.

*/

int commc_net_sendto(commc_socket_t sock, const char* host, unsigned short port, const void* buffer, size_t len) {

  struct addrinfo  hints;
  struct addrinfo* res;
  struct addrinfo* p;
  char             port_str[6];
  int              status;
  int              bytes_sent = -1;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  sprintf(port_str, "%u", port);

  status = getaddrinfo(host, port_str, &hints, &res);

  if  (status != 0) {

    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return -1;

  }

  for  (p = res; p != NULL; p = p->ai_next) {

    bytes_sent = sendto(sock, (const char*)buffer, (int)len, 0, p->ai_addr, (int)p->ai_addrlen);

    if  (bytes_sent != SOCKET_ERROR) {

      break;

    }
  }

  freeaddrinfo(res);

  if  (bytes_sent == SOCKET_ERROR) {
    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
  }

  return bytes_sent;

}

/*

         commc_net_recvfrom()
	       ---
	       receives UDP data, storing sender's address.

*/

int commc_net_recvfrom(commc_socket_t sock, void* buffer, size_t len, char* out_host, size_t host_len, unsigned short* out_port) {

  struct sockaddr_storage sender_addr;

  socklen_t  addr_len = sizeof(sender_addr);

  int  bytes_recv;

  bytes_recv = recvfrom(sock, (char*)buffer, (int)len, 0, (struct sockaddr*)&sender_addr, &addr_len);

  if  (bytes_recv == SOCKET_ERROR) {

    commc_report_error(COMMC_SYSTEM_ERROR, __FILE__, __LINE__);
    return -1;

  }

  if  (out_host && host_len > 0) {

    if  (sender_addr.ss_family == AF_INET) {

      struct sockaddr_in* ipv4 = (struct sockaddr_in*)&sender_addr;
      inet_ntop(AF_INET, &(ipv4->sin_addr), out_host, (socklen_t)host_len);

      if  (out_port) {

        *out_port = ntohs(ipv4->sin_port);

      }

    } else if  (sender_addr.ss_family == AF_INET6) {

      struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)&sender_addr;
      inet_ntop(AF_INET6, &(ipv6->sin6_addr), out_host, (socklen_t)host_len);

      if  (out_port) {

        *out_port = ntohs(ipv6->sin6_port);

      }

    } else {

      /* unknown address family */

      out_host[0] = '\0';
      if  (out_port) {

        *out_port = 0;

      }
    }
  }

  return bytes_recv;

}

/*
	==================================
             --- EOF ---
	==================================
*/
