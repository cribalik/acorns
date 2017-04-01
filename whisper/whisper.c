#include "whisper.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

int whisper_tcp_receiver_init(Whisper_TCPReceiver* r, short port) {
  return whisper_tcp_receiver_init_ex(r, port, 0);
}

int whisper_tcp_receiver_init_ex(Whisper_TCPReceiver* r, short port, int non_blocking) {
  int
    socket_fd,
    result,
    on = 1;
  struct sockaddr_in address = {0};

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) return socket_fd;
  *r = socket_fd;

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

int whisper_tcp_receiver_is_inited(Whisper_TCPReceiver r) {
  return r > 0;
}

int whisper_tcp_receiver_close(Whisper_TCPReceiver r) {
  if (r < 0) return r;
  return close(r);
}

int whisper_tcp_receiver_poll(Whisper_TCPReceiver r, Whisper_TCPConnection* c) {
  if (r < 0) return r;
  *c = accept(r, 0, 0);
  if (*c < 0) return *c;
  return 0;
}

int whisper_tcp_connection_write(Whisper_TCPConnection c, char* data, int size) {
  if (c < 0) return c;
  return write(c, data, size);
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
