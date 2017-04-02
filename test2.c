#include "whisper/whisper.h"
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

int main(int argc, char const *argv[])
{
  int err, n;
  char buf[256];
  Whisper_TCPConnection c;

  UNUSED(argc), UNUSED(argv);

  err = whisper_tcp_client(&c, "localhost", 1234);
  if (err) {
    perror("Failed to open connection");
    return 1;
  }

  for (;;) {
    n = whisper_tcp_connection_read(c, buf, sizeof(buf));
    if (n <= 0) break;
    fwrite(buf, n, 1, stdout);
    fflush(stdout);
  }
  whisper_tcp_connection_close(c);
  return 0;
}