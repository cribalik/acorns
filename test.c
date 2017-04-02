#include "whisper/whisper.h"
#include <stdio.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

int main(int argc, const char *argv[]) {
  int read, err;
  FILE* f;
  char buffer[256];
  Whisper_TCPServer receiver;
  Whisper_TCPConnection connection;

  UNUSED(argc), UNUSED(argv);

  err = whisper_tcp_server_init_ex(&receiver, 1234, 1);
  if (err) {
    perror("Couldn't initialize tcp receiver");
    return 1;
  }

  for (;;) {
    err = whisper_tcp_server_poll(receiver, &connection);
    if (err) {
      sleep(1);
      continue;
    }

    f = fopen("test.c", "r");
    if (!f)
      continue;

    for (;;) {
      read = fread(buffer, 1, sizeof(buffer), f);
      if (read <= 0) break;
      err = whisper_tcp_connection_write(connection, buffer, read);
      if (err < 0) {perror("Failed to write to socket"); break;}
    }

    whisper_tcp_connection_close(connection);
    fclose(f);
  }
  return 0;
}
