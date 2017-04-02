#define _POSIX_C_SOURCE 201112L
#include "whisper.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

void * memcpy ( void * destination, const void * source, size_t num );

int whisper_tcp_server_init(Whisper_TCPServer* r_out, short port) {
  return whisper_tcp_server_init_ex(r_out, port, 0);
}

int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, short port, int non_blocking) {
  int
    socket_fd,
    result,
    on = 1;
  struct sockaddr_in address = {0};

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) return socket_fd;
  *r_out = socket_fd;

  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (non_blocking) {
    result = fcntl(socket_fd, F_GETFL, 0);
    if (result < 0) goto after_socket;
    result = result | O_NONBLOCK;
    result = fcntl(socket_fd, F_SETFL, result);
    if (result < 0) goto after_socket;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  result = bind(socket_fd, (struct sockaddr*) &address, sizeof(address));
  if (result < 0) goto after_socket;

  result = listen(socket_fd, 20);
  if (result < 0) goto after_socket;

  return 0;

  after_socket:
  close(socket_fd);
  return result;
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

int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, short port) {
  int
    socket_fd,
    error;
  struct addrinfo *address_info;
  struct addrinfo hints = {0};

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) return socket_fd;
  *c_out = socket_fd;

  /* resolve hostname */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  error = getaddrinfo(hostname, 0, &hints, &address_info);
  if (error) goto after_socket;

  /* try all the different hostnames */
  for (; address_info; address_info = address_info->ai_next) {
    ((struct sockaddr_in*) address_info->ai_addr)->sin_port = htons(port);
    error = connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen);
    if (error == 0) break;
  }

  if (error < 0) goto after_socket;

  return 0;

  after_socket:
  close(socket_fd);
  return error;
}

int whisper_tcp_connection_write(Whisper_TCPConnection c, char* data, int size) {
  if (c < 0) return c;
  return write(c, data, size);
}

int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int size) {
  if (c < 0) return c;
  return read(c, out, size);
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
