#define DEBUG 1
#include "whisper/whisper.h"
#include <stdio.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

#if 0
void test() {
	FILE* a,b,c,d;
	int err;

	a = fopen("a.txt");
	if (a) {
		b = fopen("b.txt");
		if (b) {
			c = fopen("c.txt");
			if (c) {
				d = fopen("c.txt");
				if (d) {
					err = fread(c);
					if (!err) {
						err = fread(b);
						if (!err) {
							printf("Success");
						}
					}
					fclose(d);
				}
				fclose(c);
			}
			fclose(b);
		}
		fclose(a);
	}

	return err;
}

void test2() {
	FILE* a,b,c,d;
	int err;

	a = fopen("a.txt");
	if (!a) return;

	b = fopen("b.txt");
	if (!b) goto after_a;

	c = fopen("c.txt");
	if (!c) goto after_b;

	d = fopen("d.txt");
	if (!d) goto after_c;

	err = fread(c);
	if (err) goto after_d

	err = fread(b);
	if (err) goto after_d;

	printf("Success");

	after_d: fclose(d);
	after_c: fclose(c);
	after_b: fclose(b);
	after_a: fclose(a);
}

void test3() {
	FILE* a,b,c,d;
	int err;

	a = fopen("a.txt");
	if (!a) return;

	b = fopen("b.txt");
	if (!b) {
		fclose(a);
		return;
	}

	c = fopen("c.txt");
	if (!c) {
		fclose(a);
		fclose(b);
		return;
	}

	d = fopen("c.txt");
	if (!d) {
		fclose(a);
		fclose(b);
		fclose(c);
		return;
	}

	err = fread(c);
	if (err) {
		fclose(a);
		fclose(b);
		fclose(c);
		fclose(d);
		return;
	}

	err = fread(b);
	if (err) {
		fclose(a);
		fclose(b);
		fclose(c);
		fclose(d);
		return;
	}

	printf("Success");
}
#endif

int main(int argc, const char *argv[]) {
	Whisper_TCPServer server;
	Whisper_TCPConnection connection;
	(void)argc, (void)argv;

	whisper_tcp_server_init(&server, 1234);

	for(;;) {
		whisper_tcp_server_poll(server, &connection);
		whisper_tcp_connection_send(connection, "hello world", 12);
		whisper_tcp_connection_close(connection);
	}
	return 0;
}

void client() {
	Whisper_TCPConnection conn;
	whisper_tcp_client(&conn, "127", 1234);
	whisper_tcp_connection_send(conn, "hej", 3);
	whisper_tcp_connection_close(conn);
}