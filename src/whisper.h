/* TODO: whisper_errno with messages */

#ifdef _MSC_VER
  typedef __int8 whisper_i8;
  typedef __int16 whisper_i16;
  typedef __int32 whisper_i32;
  typedef __int64 whisper_i64;
  typedef unsigned __int8 whisper_u8;
  typedef unsigned __int16 whisper_u16;
  typedef unsigned __int32 whisper_u32;
  typedef unsigned __int64 whisper_u64;
#else
  /* let's hope stdint has us covered */
  #include <stdint.h>
  typedef int8_t whisper_i8;
  typedef int16_t whisper_i16;
  typedef int32_t whisper_i32;
  typedef int64_t whisper_i64;
  typedef uint8_t whisper_u8;
  typedef uint16_t whisper_u16;
  typedef uint32_t whisper_u32;
  typedef uint64_t whisper_u64;
#endif
#ifndef WHISPER_H
#define WHISPER_H

#ifndef WHISPER_NO_STATIC
  #define WHISPER__CALL static
#else
  #define WHISPER__CALL
#endif

#ifdef __linux__
  #define WHISPER__HANDLE int
#elif defined(_MSC_VER)
  #include <ws2tcpip.h>
  #define WHISPER__HANDLE SOCKET
#endif

typedef WHISPER__HANDLE Whisper_TCPServer;
typedef WHISPER__HANDLE Whisper_TCPConnection;
WHISPER__CALL int whisper_errno;

/* init/close */
WHISPER__CALL int whisper_init();
WHISPER__CALL int whisper_close();

/* server */
WHISPER__CALL int whisper_tcp_server_init(Whisper_TCPServer *server, unsigned short port);
WHISPER__CALL int whisper_tcp_server_init_ex(Whisper_TCPServer *server, unsigned short port, int non_blocking);
WHISPER__CALL int whisper_tcp_server_poll(Whisper_TCPServer server, Whisper_TCPConnection *connection);
WHISPER__CALL int whisper_tcp_server_close(Whisper_TCPServer r);

/* client */
WHISPER__CALL int whisper_tcp_client(Whisper_TCPConnection *c_out, const char *hostname, unsigned short port);

/* connection */
WHISPER__CALL int whisper_tcp_connection_write(Whisper_TCPConnection c, const void *data, int size);
WHISPER__CALL int whisper_tcp_connection_read(Whisper_TCPConnection c, void *out, int num_bytes);
WHISPER__CALL int whisper_tcp_connection_send(Whisper_TCPConnection c, const void *data, int num_bytes);
WHISPER__CALL int whisper_tcp_connection_receive(Whisper_TCPConnection c, void *out, int num_bytes);
WHISPER__CALL int whisper_tcp_connection_flush(Whisper_TCPConnection c);
WHISPER__CALL int whisper_tcp_connection_close(Whisper_TCPConnection c);
WHISPER__CALL int whisper_tcp_wait(Whisper_TCPServer *server, Whisper_TCPConnection *connections, int num_connections, int stride, char *connections_ready_out, int out_stride, char *server_ready_out);
WHISPER__CALL int whisper_tcp_canwait(Whisper_TCPConnection c);

WHISPER__CALL whisper_u64 whisper_hton64(whisper_u64 x);
WHISPER__CALL whisper_u32 whisper_hton32(whisper_u32 x);
WHISPER__CALL whisper_u16 whisper_hton16(whisper_u16 x);

WHISPER__CALL whisper_u64 whisper_ntoh64(whisper_u64 x);
WHISPER__CALL whisper_u32 whisper_ntoh32(whisper_u32 x);
WHISPER__CALL whisper_u16 whisper_ntoh16(whisper_u16 x);

enum {
  WHISPER_EINVAL = -1
};


/* DOCUMENTATION */

/* int whisper_tcp_server_init(Whisper_TCPServer* r_out, unsigned short port);
 *
 * Bind a tcp server to a port
 */

/* int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, unsigned short port, int non_blocking);
 * 
 * Bind a tcp server to a port
 * If non_blocking is used, then calls like whisper_tcp_server_poll are non-blocking
 */

/* int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c);
 * 
 * Listen for a connection
 * Returns a nonzero number on failure
 * If non_blocking is set, then this will fail if there are no waiting connections at the time of polling
 *
 * On success, the connection parameter can now be used
 */

/* int whisper_tcp_server_close(Whisper_TCPServer r);
 * 
 * Close a server, freeing its resources
 */

/* int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, unsigned short port);
 * 
 * Create a tcp connection to a host and port
 *
 * The format of hostname can be either be a string representing the ip address or the name of the host
 * On unix, getaddrinfo is used to perform the DNS lookup
 */

/* int whisper_tcp_connection_write(Whisper_TCPConnection c, const void *data, int size);
 * 
 * Write bytes to connection.
 * Returns the number of bytes written. This value may be less than `size`.
 * On error, returns a negative value.
 * If you want to ensure a number of bytes are written, use the convenience function whisper_tcp_connection_send
 *
 * The buffer is not guaranteed to be flushed by this call.
 * To flush the buffer, use whisper_tcp_connection_flush (or use whisper_tcp_connection_send)
 */

/* int whisper_tcp_connection_read(Whisper_TCPConnection c, void *out, int num_bytes);
 * 
 * Read bytes from connection.
 * Returns the number of bytes read. The number of bytes read may be less than num_bytes.
 * on error, returns a negative value.
 * If you want to ensure a number of bytes are read, use the convenience function whisper_tcp_connection_receive
 */

/* int whisper_tcp_connection_send(Whisper_TCPConnection c, const void *data, int num_bytes);
 * 
 * Write `num_bytes` bytes to connection and flushes the buffer
 * Returns the number of bytes written. If return value is less than `num_bytes`, an error occured
 */

/** int whisper_tcp_connection_receive(Whisper_TCPConnection c, void *out, int num_bytes);
  * 
  * Read `num_bytes` bytes from connection
  * Returns the number of bytes read. If return value is less than `num_bytes`, an error occured
  */

/** int whisper_tcp_connection_flush(Whisper_TCPConnection c);
  * 
  * Flush the buffer of the connection, guaranteeing that any buffered bytes written by whisper_tcp_connection_write are sent
  * Returns a nonzero value on error
  */

/* int whisper_tcp_connection_close(Whisper_TCPConnection c);
  * 
  * Closes a connection, freeing its resources
  */

/* int whisper_tcp_wait(Whisper_TCPServer *server, Whisper_TCPConnection *connections, int num_connections, int stride, char *connections_ready_out, int out_stride, char *server_ready_out);
  *
  * Wait on a server and multiple connections until either the server is ready to accept a connection or the connections are ready to receive data
  *
  * connections_ready_out - an array of size num_connections, where 1 indicates that the connection is ready to receive data, 0 otherwise
  * server_ready_out - will be set to 1 if server is ready to receive connections, 0 otherwise
  */

#endif /* WHISPER_H */

#ifdef WHISPER_IMPLEMENTATION



/** GENERIC IMPLEMENTATION **/

WHISPER__CALL int whisper_errno;

#if DEBUG
  #define WHISPER_DEBUG(STMT) do {STMT;} while(0)
#else
  #define WHISPER_DEBUG(STMT)
#endif

int whisper_tcp_server_init(Whisper_TCPServer* r_out, unsigned short port) {
  return whisper_tcp_server_init_ex(r_out, port, 0);
}

WHISPER__CALL whisper_u64 whisper_hton64(whisper_u64 x) {
  whisper_u64 r;
  unsigned char *p = (unsigned char*)&r;

  p[0] = (x >> 56) & 0xFF;
  p[1] = (x >> 48) & 0xFF;
  p[2] = (x >> 40) & 0xFF;
  p[3] = (x >> 32) & 0xFF;
  p[4] = (x >> 24) & 0xFF;
  p[5] = (x >> 16) & 0xFF;
  p[6] = (x >> 8) & 0xFF;
  p[7] = x & 0xFF;
  return r;
}

WHISPER__CALL whisper_u32 whisper_hton32(whisper_u32 x) {
  whisper_u32 r;
  unsigned char *p = (unsigned char*)&r;

  p[0] = (x >> 24) & 0xFF;
  p[1] = (x >> 16) & 0xFF;
  p[2] = (x >> 8) & 0xFF;
  p[3] = x & 0xFF;
  return r;
}


WHISPER__CALL unsigned short whisper_hton16(unsigned short x) {
  unsigned short r;
  unsigned char *p = (unsigned char*)&r;

  p[0] = (x >> 8) & 0xFF;
  p[1] = x & 0xFF;
  return r;
}

WHISPER__CALL whisper_u64 whisper_ntoh64(whisper_u64 x) {
  unsigned char *p = (unsigned char*)&x;
  return
    (whisper_u64)p[0] << 56 |
    (whisper_u64)p[1] << 48 |
    (whisper_u64)p[2] << 40 |
    (whisper_u64)p[3] << 32 |
    (whisper_u64)p[4] << 24 |
    (whisper_u64)p[5] << 16 |
    (whisper_u64)p[6] << 8 |
    (whisper_u64)p[7];
}

WHISPER__CALL whisper_u32 whisper_ntoh32(whisper_u32 x) {
  unsigned char *p = (unsigned char*)&x;
  return
    (whisper_u32)p[0] << 24 |
    (whisper_u32)p[1] << 16 |
    (whisper_u32)p[2] << 8 |
    (whisper_u32)p[3];
}

WHISPER__CALL unsigned short whisper_ntoh16(unsigned short x) {
  unsigned char *p = (unsigned char*)&x;
  return
    (unsigned short)p[0] << 8 |
    (unsigned short)p[1];
}



/** LINUX IMPLEMENTATION **/
#ifdef __linux__

/* TODO: support IPv6 :) */

/* We use getaddrinfo instead of gethostname */
#if _POSIX_C_SOURCE < 201112L
#error "getaddrinfo not available, increase _POSIX_C_SOURCE version"
#endif

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <alloca.h>


/* Some debugging stuff */
#ifdef DEBUG
#include <stdio.h>
#endif

WHISPER__CALL int whisper_init() {
  return 0;
}

WHISPER__CALL int whisper_close() {
  return 0;
}

WHISPER__CALL int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, unsigned short port, int non_blocking) {
  int
    socket_fd,
    err,
    on = 1;
  struct sockaddr_in address = {0};

  *r_out = socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
    return socket_fd;
  *r_out = socket_fd;

  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (non_blocking) {
    err = fcntl(socket_fd, F_GETFL, 0);
    if (err < 0)
      goto err_socket;
    err = err | O_NONBLOCK;
    err = fcntl(socket_fd, F_SETFL, err);
    if (err < 0)
      goto err_socket;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  err = bind(socket_fd, (struct sockaddr*) &address, sizeof(address));
  if (err < 0)
    goto err_socket;

  err = listen(socket_fd, 20);
  if (err < 0)
    goto err_socket;

  return 0;

  err_socket:
  close(socket_fd);
  return err;
}

WHISPER__CALL int whisper_tcp_server_close(Whisper_TCPServer r) {
  if (r < 0) return r;
  return close(r);
}

WHISPER__CALL int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c) {
  if (r < 0) return r;
  *c = accept(r, 0, 0);
  if (*c < 0) return *c;
  return 0;
}

WHISPER__CALL int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, unsigned short port) {
  int
    socket_fd,
    error;
  struct addrinfo *address_info_head, *address_info;
  struct addrinfo hints = {0};
  struct sockaddr_in address;

  *c_out = socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
    return socket_fd;

  /* resolve hostname */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  error = getaddrinfo(hostname, 0, &hints, &address_info_head);
  if (error) goto err;

  /* try all the different hosts */
  for (address_info = address_info_head; address_info; address_info = address_info->ai_next) {
    address = *(struct sockaddr_in*) address_info->ai_addr;
    address.sin_port = htons(port);

    WHISPER_DEBUG(printf("Trying to connect to %s:%i ... ", inet_ntoa(address.sin_addr), ntohs(address.sin_port)));

    error = connect(socket_fd, (struct sockaddr*) &address, sizeof(address));

    WHISPER_DEBUG(
      if (error) perror("Failed");
      else printf("Success\n"););

    if (!error) break;
  }

  freeaddrinfo(address_info_head);

  if (!address_info) goto err;

  return 0;

  err:
  close(socket_fd);
  return error;
}

WHISPER__CALL int whisper_tcp_connection_write(Whisper_TCPConnection c, const void *data, int size) {
  if (c < 0) return c;
  return write(c, data, size);
}

WHISPER__CALL int whisper_tcp_connection_send(Whisper_TCPConnection c, const void *data, int num_bytes) {
  int num_read;
  const char* p;
  if (c < 0)
    return 0;
  p = data;
  for (;;) {
    num_read = write(c, p, (char*)data + num_bytes - p);
    if (num_read < 0)
      return -1;
    p += num_read;
    if (p >= (char*)data + num_bytes) break;
  }
  if (p - (char*)data == num_bytes) whisper_tcp_connection_flush(c);
  return p - (char*)data;
}

WHISPER__CALL int whisper_tcp_connection_receive(Whisper_TCPConnection c, void *out, int num_bytes) {
  int num_written;
  char* p;
  if (c < 0 || num_bytes <= 0)
    return 0;
  p = out;
  for (;;) {
    num_written = read(c, p, (char*)out + num_bytes - p);
    if (num_written < 0)
      return -1;
    p += num_written;
    if (p >= (char*)out + num_bytes)
      break;
  }
  return p - (char*)out;
}

WHISPER__CALL int whisper_tcp_connection_read(Whisper_TCPConnection c, void *out, int num_bytes) {
  if (c < 0) return c;
  return read(c, out, num_bytes);
}

WHISPER__CALL int whisper_tcp_connection_flush(Whisper_TCPConnection c) {
  if (c < 0) return c;
  return fsync(c);
}

WHISPER__CALL int whisper_tcp_connection_close(Whisper_TCPConnection c) {
  if (c < 0) return c;
  whisper_tcp_connection_flush(c);
  return close(c);
}

WHISPER__CALL int whisper_tcp_canwait(Whisper_TCPConnection c) {
  return c > 0 && c < FD_SETSIZE;
}

WHISPER__CALL int whisper_tcp_wait(Whisper_TCPServer *server, Whisper_TCPConnection *connections, int num_connections, int stride, char *connections_ready_out, int out_stride, char *server_ready_out) {
  int i, err;
  fd_set fdset;
  int nfds;

  FD_ZERO(&fdset);
  nfds = 0;

  if (server_ready_out)
    *server_ready_out = 0;
  for (i = 0; i < num_connections; ++i)
    *(char*)((char*)connections_ready_out + i*out_stride) = 0;

  /* add server */
  if (server) {
    if (*server > nfds)
      nfds = *server;
    if (*server >= FD_SETSIZE)
      return whisper_errno = WHISPER_EINVAL;
    FD_SET(*server, &fdset);
  }

  /* add connections */
  for (i = 0; i < num_connections; ++i) {
    Whisper_TCPConnection *c;

    c = (Whisper_TCPConnection*)((char*)connections + i*stride);
    if (*c > nfds)
      nfds = *c;
    if (*c >= FD_SETSIZE)
      return whisper_errno = WHISPER_EINVAL;
    FD_SET(*c, &fdset);
  }

  err = select(nfds+1, &fdset, 0, 0, 0);
  if (err == -1)
    return 1;

  if (server_ready_out)
    *server_ready_out = FD_ISSET((*server), &fdset);
  if (connections_ready_out)
    for (i = 0; i < num_connections; ++i) {
      Whisper_TCPConnection *c = (Whisper_TCPConnection*) ((char*)connections + i*stride);
      char *out = connections_ready_out + i*out_stride;

      *out = FD_ISSET(*c, &fdset);
    }
  return 0;
}







/** WINDOWS IMPLEMENTATION **/
#elif defined(_MSC_VER)

#pragma comment(lib, "Ws2_32.lib")

WHISPER__CALL int whisper_init() {
  WSADATA wsadata;
  return WSAStartup(1, &wsadata);
}

WHISPER__CALL int whisper_close() {
  return WSACleanup();
}

#define whisper_tcp_connection_flush(c) 0
WHISPER__CALL int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, unsigned short port, int non_blocking) {
  SOCKET sock;
  int err,
      reuse_address = 1;
  struct sockaddr_in address = {0};

  *r_out = sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET)
    return WSAGetLastError();

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_address, sizeof(reuse_address));

  if (non_blocking) {
    err = ioctlsocket(sock, FIONBIO, &non_blocking);
    if (err)
      goto err_socket;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  err = bind(sock, (struct sockaddr*) &address, sizeof(address));
  if (err)
    goto err_socket;

  /* TODO: let user specify backlog */
  err = listen(sock, 20);
  if (err)
    goto err_socket;

  return 0;

  err_socket:
  closesocket(sock);
  return err;
}

WHISPER__CALL int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c) {
  if (r == INVALID_SOCKET)
    return 1;
  *c = accept(r, 0, 0);
  if (*c == INVALID_SOCKET)
    return 1;
  return 0;
}

WHISPER__CALL int whisper_tcp_server_close(Whisper_TCPServer r) {
  if (r == INVALID_SOCKET)
    return 1;
  return closesocket(r);
}

WHISPER__CALL int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, unsigned short port) {
  SOCKET socket_fd;
  int error = 1;
  struct addrinfo *address_info_head, *address_info;
  struct addrinfo hints = {0};
  struct sockaddr_in address;

  *c_out = socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd == INVALID_SOCKET)
    return 1;

  /* resolve hostname */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  error = getaddrinfo(hostname, 0, &hints, &address_info_head);
  if (error)
    goto err;

  /* try all the different hosts */
  for (address_info = address_info_head; address_info; address_info = address_info->ai_next) {
    char address_buffer[16];
    address = *(struct sockaddr_in*) address_info->ai_addr;
    address.sin_port = htons(port);

    WHISPER_DEBUG(printf("Trying to connect to %s:%i ... ", InetNtop(AF_INET, &address.sin_addr, address_buffer, sizeof(address_buffer)), ntohs(address.sin_port)));

    error = connect(socket_fd, (struct sockaddr*) &address, sizeof(address));

    WHISPER_DEBUG(
      if (error)
        printf("Failed\n");
      else printf("Success\n"););

    if (!error)
      break;
  }

  freeaddrinfo(address_info_head);

  if (!address_info) {
    error = 1;
    goto err;
  }

  return 0;

  err:
  closesocket(socket_fd);
  /* TODO: sensical return codes */
  return error;
}

WHISPER__CALL int whisper_tcp_connection_write(Whisper_TCPConnection c, const void *data, int size) {
  int err;

  if (c == INVALID_SOCKET)
    return -1;
  err = send(c, data, size, 0);
  if (err == SOCKET_ERROR)
    return -1;
  return err;
}

WHISPER__CALL int whisper_tcp_connection_read(Whisper_TCPConnection c, void *out, int num_bytes) {
  int err;

  if (c == INVALID_SOCKET)
    return -1;
  err = recv(c, out, num_bytes, 0);
  if (err == SOCKET_ERROR)
    return -1;
  return err;
}

WHISPER__CALL int whisper_tcp_connection_send(Whisper_TCPConnection c, const void *data, int num_bytes) {
  int num_read, err;
  const char* p;
  if (c == INVALID_SOCKET || num_bytes <= 0)
    return 0;
  p = data;
  for (;;) {
    num_read = send(c, p, (char*)data + num_bytes - p, 0);
    if (num_read == SOCKET_ERROR)
      return 0;
    p += num_read;
    if (p >= (char*)data + num_bytes)
      break;
  }
  if (p - (char*)data == num_bytes) {
    err = whisper_tcp_connection_flush(c);
    if (err)
      return 0;
  }
  return p - (char*)data;
}

WHISPER__CALL int whisper_tcp_connection_receive(Whisper_TCPConnection c, void *out, int num_bytes) {
  int num_written;
  char* p;
  if (c == INVALID_SOCKET || num_bytes <= 0)
    return 0;
  p = out;
  for (;;) {
    num_written = recv(c, p, (char*)out + num_bytes - p, 0);
    if (num_written < 0)
      return 0;
    p += num_written;
    if (p >= (char*)out + num_bytes)
      break;
  }
  return p - (char*)out;
}

WHISPER__CALL int whisper_tcp_connection_close(Whisper_TCPConnection c) {
  int err;
  if (c == INVALID_SOCKET)
    return 1;
  err = whisper_tcp_connection_flush(c);
  if (err)
    return 1;
  return closesocket(c);
}





/** TODO: MAC IMPLEMENTATION **/
#elif defined(__APPLE__)
  #error Platform not supported
#else
  #error "Unknown platform"
#endif

typedef char whisper_assert_long_is_64bit[sizeof(whisper_i64) == 8 ? 1:-1];
typedef char whisper_assert_int_is_32bit[sizeof(whisper_i32) == 4 ? 1:-1];
typedef char whisper_assert_int_is_16bit[sizeof(whisper_i16) == 2 ? 1:-1];
typedef char whisper_assert_long_is_8bit[sizeof(whisper_i8) == 1 ? 1:-1];

#endif /* WHISPER_IMPLEMENTATION */
