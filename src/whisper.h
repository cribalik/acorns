#ifndef WHISPER_H
#define WHISPER_H

/* TODO: whisper_errno with messages */

#if !defined(__linux__) && !defined(__APPLE__) && !defined(_MSC_VER)
#error "Unknown platform"
#endif

/** LINUX HEADER **/
#ifdef __linux__

typedef int Whisper_TCPServer;
typedef int Whisper_TCPConnection;

/** WINDOWS HEADER **/
#elif defined(_MSC_VER)

#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
typedef SOCKET Whisper_TCPServer;
typedef SOCKET Whisper_TCPConnection;

#endif


int whisper_tcp_server_init(Whisper_TCPServer *server, unsigned short port);
int whisper_tcp_server_init_ex(Whisper_TCPServer *server, unsigned short port, int non_blocking);
int whisper_tcp_server_poll(Whisper_TCPServer server, Whisper_TCPConnection *connection);
int whisper_tcp_server_close(Whisper_TCPServer r);

int whisper_tcp_client(Whisper_TCPConnection *c_out, const char *hostname, unsigned short port);

int whisper_tcp_connection_write(Whisper_TCPConnection c, const char *data, int size);
int whisper_tcp_connection_read(Whisper_TCPConnection c, char *out, int num_bytes);
int whisper_tcp_connection_send(Whisper_TCPConnection c, const char *data, int num_bytes);
int whisper_tcp_connection_receive(Whisper_TCPConnection c, char *out, int num_bytes);
int whisper_tcp_connection_flush(Whisper_TCPConnection c);
int whisper_tcp_connection_close(Whisper_TCPConnection c);

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

/* int whisper_tcp_connection_write(Whisper_TCPConnection c, const char* data, int size);
 * 
 * Write bytes to connection.
 * Returns the number of bytes written. This value may be less than `size`.
 * On error, returns a negative value.
 * If you want to ensure a number of bytes are written, use the convenience function whisper_tcp_connection_send
 *
 * The buffer is not guaranteed to be flushed by this call.
 * To flush the buffer, use whisper_tcp_connection_flush (or use whisper_tcp_connection_send)
 */

/* int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int num_bytes);
 * 
 * Read bytes from connection.
 * Returns the number of bytes read. The number of bytes read may be less than num_bytes.
 * on error, returns a negative value.
 * If you want to ensure a number of bytes are read, use the convenience function whisper_tcp_connection_receive
 */

/* int whisper_tcp_connection_send(Whisper_TCPConnection c, const char* data, int num_bytes);
 * 
 * Write `num_bytes` bytes to connection and flushes the buffer
 * Returns the number of bytes written. If return value is less than `num_bytes`, an error occured
 */

/** int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes);
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

#endif /* WHISPER_H */

#ifdef WHISPER_IMPLEMENTATION



/** GENERIC IMPLEMENTATION **/

#if DEBUG
  #define WHISPER_DEBUG(STMT) do {STMT;} while(0)
#else
  #define WHISPER_DEBUG(STMT)
#endif

int whisper_tcp_server_init(Whisper_TCPServer* r_out, unsigned short port) {
  return whisper_tcp_server_init_ex(r_out, port, 0);
}




/** LINUX IMPLEMENTATION **/
#ifdef __linux__

/* TODO: support IPv6 :) */

/* We use getaddrinfo instead of gethostname */
#if _POSIX_C_SOURCE < 201112L
#error getaddrinfo not available, increase _POSIX_C_SOURCE version
#endif

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>


/* Some debugging stuff */
#ifdef DEBUG
#include <stdio.h>
#endif


int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, unsigned short port, int non_blocking) {
  int
    socket_fd,
    err,
    on = 1;
  struct sockaddr_in address = {0};

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
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

int whisper_tcp_server_close(Whisper_TCPServer r) {
  if (r < 0) return r;
  return close(r);
}

int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c) {
  if (r < 0) return r;
  *c = accept(r, 0, 0);
  if (*c < 0) return *c;
  return 0;
}

int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, unsigned short port) {
  int
    socket_fd,
    error;
  struct addrinfo *address_info_head, *address_info;
  struct addrinfo hints = {0};
  struct sockaddr_in address;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) return socket_fd;
  *c_out = socket_fd;

  /* resolve hostname */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  error = getaddrinfo(hostname, 0, &hints, &address_info_head);
  if (error) goto err;

  /* try all the different hosts */
  for (address_info = address_info_head; address_info; address_info = address_info->ai_next) {
    address = *(struct sockaddr_in*) address_info->ai_addr;
    address.sin_port = htons(port);

    WHISPER_DEBUG(printf("Trying to connect to %s %i ... ", inet_ntoa(address.sin_addr), ntohs(address.sin_port)));

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

int whisper_tcp_connection_write(Whisper_TCPConnection c, const char* data, int size) {
  if (c < 0) return c;
  return write(c, data, size);
}

int whisper_tcp_connection_send(Whisper_TCPConnection c, const char* data, int num_bytes) {
  int num_read;
  const char* p;
  if (c < 0) return c;
  p = data;
  for (;;) {
    num_read = write(c, p, data + num_bytes - p);
    if (num_read < 0)
      return -1;
    p += num_read;
    if (p >= data + num_bytes) break;
  }
  if (p - data == num_bytes) whisper_tcp_connection_flush(c);
  return p - data;
}

int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes) {
  int num_written;
  char* p;
  if (c < 0)
    return c;
  p = out;
  for (;;) {
    num_written = read(c, p, out + num_bytes - p);
    if (num_written < 0)
      return -1;
    p += num_written;
    if (p >= out + num_bytes)
      break;
  }
  return p - out;
}

int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int num_bytes) {
  if (c < 0) return c;
  return read(c, out, num_bytes);
}

int whisper_tcp_connection_flush(Whisper_TCPConnection c) {
  if (c < 0) return c;
  return fsync(c);
}

int whisper_tcp_connection_close(Whisper_TCPConnection c) {
  if (c < 0) return c;
  whisper_tcp_connection_flush(c);
  return close(c);
}







/** WINDOWS IMPLEMENTATION **/
#elif defined(_MSC_VER)

#define whisper_tcp_connection_flush(c) 0

int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, unsigned short port, int non_blocking) {
  SOCKET sock;
  int err,
      reuse_address = 1;
  struct sockaddr_in address = {0};

  *r_out = sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET) {
    return 1;
  }

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

int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c) {
  if (r == INVALID_SOCKET)
    return 1;
  *c = accept(r, 0, 0);
  if (*c == INVALID_SOCKET)
    return 1;
  return 0;
}

int whisper_tcp_server_close(Whisper_TCPServer r) {
  if (r == INVALID_SOCKET)
    return 1;
  return closesocket(r);
}

int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, unsigned short port) {
  SOCKET socket_fd;
  int error;
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

    WHISPER_DEBUG(printf("Trying to connect to %s %i ... ", InetNtop(AF_INET, &address.sin_addr, address_buffer, sizeof(address_buffer)), ntohs(address.sin_port)));

    error = connect(socket_fd, (struct sockaddr*) &address, sizeof(address));

    WHISPER_DEBUG(
      if (error)
        perror("Failed");
      else printf("Success\n"););

    if (!error)
      break;
  }

  freeaddrinfo(address_info_head);

  if (!address_info)
    goto err;

  return 0;

  err:
  closesocket(socket_fd);
  /* TODO: sensical return codes */
  return 1;
}

int whisper_tcp_connection_write(Whisper_TCPConnection c, const char* data, int size) {
  int err;

  if (c == INVALID_SOCKET)
    return -1;
  err = send(c, data, size, 0);
  if (err == SOCKET_ERROR)
    return -1;
  return err;
}

int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int num_bytes) {
  int err;

  if (c == INVALID_SOCKET)
    return -1;
  err = recv(c, out, num_bytes, 0);
  if (err == SOCKET_ERROR)
    return -1;
  return err;
}

int whisper_tcp_connection_send(Whisper_TCPConnection c, const char* data, int num_bytes) {
  int num_read, err;
  const char* p;
  if (c == INVALID_SOCKET)
    return -1;
  p = data;
  for (;;) {
    num_read = send(c, p, data + num_bytes - p, 0);
    if (num_read == SOCKET_ERROR)
      return -1;
    p += num_read;
    if (p >= data + num_bytes)
      break;
  }
  if (p - data == num_bytes) {
    err = whisper_tcp_connection_flush(c);
    if (err)
      return -1;
  }
  return p - data;
}

int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes) {
  int num_written;
  char* p;
  if (c == INVALID_SOCKET)
    return -1;
  p = out;
  for (;;) {
    num_written = recv(c, p, out + num_bytes - p, 0);
    if (num_written < 0)
      return -1;
    p += num_written;
    if (p >= out + num_bytes)
      break;
  }
  return p - out;
}

int whisper_tcp_connection_close(Whisper_TCPConnection c) {
  int err;
  if (c == INVALID_SOCKET)
    return 1;
  err = whisper_tcp_connection_flush(c);
  if (err)
    return 1;
  return closesocket(c);
}





/** TODO: MAC IMPLEMENTATION **/
#else
#error Platform not supported
#endif /* WHISPER_IMPLEMENTATION */

#endif