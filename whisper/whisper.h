#ifndef WHISPER_H
#define WHISPER_H

typedef int Whisper_TCPServer;
typedef int Whisper_TCPConnection;

int whisper_tcp_server_init(Whisper_TCPServer* r_out, short port);
int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, short port, int non_blocking);
int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c);
int whisper_tcp_server_close(Whisper_TCPServer r);

int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, short port);

int whisper_tcp_connection_write(Whisper_TCPConnection c, char* data, int size);
int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int size);
int whisper_tcp_connection_flush(Whisper_TCPConnection c);
int whisper_tcp_connection_close(Whisper_TCPConnection c);

#endif