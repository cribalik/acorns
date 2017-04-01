#ifndef WHISPER_H
#define WHISPER_H

typedef int Whisper_TCPReceiver;
typedef int Whisper_TCPConnection;

int whisper_tcp_receiver_init(Whisper_TCPReceiver* r, short port);
int whisper_tcp_receiver_init_ex(Whisper_TCPReceiver* r, short port, int non_blocking);
int whisper_tcp_receiver_poll(Whisper_TCPReceiver r, Whisper_TCPConnection* c);
int whisper_tcp_receiver_close(Whisper_TCPReceiver r);

int whisper_tcp_connection_write(Whisper_TCPConnection c, char* data, int size);
int whisper_tcp_connection_flush(Whisper_TCPConnection c);
int whisper_tcp_connection_close(Whisper_TCPConnection c);

#endif