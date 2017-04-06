#ifndef WHISPER_H
#define WHISPER_H

typedef int Whisper_TCPServer;
typedef int Whisper_TCPConnection;

int whisper_tcp_server_init(Whisper_TCPServer* r_out, short port);
int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, short port, int non_blocking);
int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c);
int whisper_tcp_server_close(Whisper_TCPServer r);

int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, short port);

int whisper_tcp_connection_write(Whisper_TCPConnection c, const char* data, int size);
int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int num_bytes);
int whisper_tcp_connection_send(Whisper_TCPConnection c, const char* data, int num_bytes);
int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes);
int whisper_tcp_connection_flush(Whisper_TCPConnection c);
int whisper_tcp_connection_close(Whisper_TCPConnection c);

/* DOCUMENTATION */

/**
* int whisper_tcp_server_init(Whisper_TCPServer* r_out, short port);
*
* Bind a tcp server to a port
*/

/**
* int whisper_tcp_server_init_ex(Whisper_TCPServer* r_out, short port, int non_blocking);
* 
* Bind a tcp server to a port
* If non_blocking is used, then calls like whisper_tcp_server_poll are non-blocking
*/

/**
* int whisper_tcp_server_poll(Whisper_TCPServer r, Whisper_TCPConnection* c);
* 
* Listen for a connection
* Returns a nonzero number on failure
* If non_blocking is set, then this will fail if there are no waiting connections at the time of polling
*
* On success, the connection parameter can now be used
*/

/**
* int whisper_tcp_server_close(Whisper_TCPServer r);
* 
* Close a server, freeing its resources
*/

/**
* int whisper_tcp_client(Whisper_TCPConnection* c_out, const char* hostname, short port);
* 
* Create a tcp connection to a host and port
*
* The format of hostname can be either be a string representing the ip address or the name of the host
* On unix, getaddrinfo is used to perform the DNS lookup
*/

/**
* int whisper_tcp_connection_write(Whisper_TCPConnection c, const char* data, int size);
* 
* Write bytes to connection.
* Returns the number of bytes written. This value may be less than `size`.
* If you want to ensure a number of bytes are written, use the convenience function whisper_tcp_connection_send
*
* The buffer is not guaranteed to be flushed by this call.
* To flush the buffer, use whisper_tcp_connection_flush (or use whisper_tcp_connection_send)
*/

/**
* int whisper_tcp_connection_read(Whisper_TCPConnection c, char* out, int num_bytes);
* 
* Read bytes from connection.
* Returns the number of bytes read. The number of bytes read may be less than num_bytes.
* If you want to ensure a number of bytes are read, use the convenience function whisper_tcp_connection_receive
*/

/**
* int whisper_tcp_connection_send(Whisper_TCPConnection c, const char* data, int num_bytes);
* 
* Write `num_bytes` bytes to connection and flushes the buffer
* Returns the number of bytes written. If return value is less than `num_bytes`, an error occured
*/

/**
* int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes);
* 
* Read `num_bytes` bytes from connection
* Returns the number of bytes read. If return value is less than `num_bytes`, an error occured
*/

/**
* int whisper_tcp_connection_flush(Whisper_TCPConnection c);
* 
* Flush the buffer of the connection, guaranteeing that any buffered bytes written by whisper_tcp_connection_write are sent
* Returns a nonzero value on error
*/

/**
* int whisper_tcp_connection_close(Whisper_TCPConnection c);
* 
* Closes a connection, freeing its resources
*/

#endif