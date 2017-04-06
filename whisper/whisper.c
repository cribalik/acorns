/* TODO: support IPv6 :) */

/* We use getaddrinfo instead of gethostname */
#define _POSIX_C_SOURCE 201112L
#include "whisper.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>


/* Some debugging stuff */
#if DEBUG
	#include <stdio.h>
	#define WHISPER_DEBUG(STMT) do {STMT;} while(0)
#else
	#define WHISPER_DEBUG(STMT) while(0) {STMT;}
#endif

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

	after_socket: close(socket_fd);
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
	struct sockaddr_in address;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) return socket_fd;
	*c_out = socket_fd;

	/* resolve hostname */
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(hostname, 0, &hints, &address_info);
	if (error) goto after_socket;

	/* try all the different hosts */
	for (; address_info; address_info = address_info->ai_next) {
		address = *(struct sockaddr_in*) address_info->ai_addr;
		address.sin_port = htons(port);

		WHISPER_DEBUG(printf("Trying to connect to %s %i ... ", inet_ntoa(address.sin_addr), ntohs(address.sin_port)));

		error = connect(socket_fd, (struct sockaddr*) &address, sizeof(address));

		WHISPER_DEBUG( if (error) perror("Failed"); else printf("Success\n"); );

		if (!error) break;
	}

	after_socket: close(socket_fd);
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
		if (num_read < 0) break;
		p += num_read;
		if (p >= data + num_bytes) break;
	}
	if (p - data == num_bytes) whisper_tcp_connection_flush(c);
	return p - data;
}

int whisper_tcp_connection_receive(Whisper_TCPConnection c, char* out, int num_bytes) {
	int num_written;
	char* p;
	if (c < 0) return c;
	p = out;
	for (;;) {
		num_written = read(c, p, out + num_bytes - p);
		if (num_written < 0) break;
		p += num_written;
		if (p >= out + num_bytes) break;
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
