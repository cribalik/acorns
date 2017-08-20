#if 0
#define _XOPEN_SOURCE 500
#else
#define _POSIX_C_SOURCE 201112L
#endif

#define DEBUG 1
#include "utils.h"
#define TEXT_IMPLEMENTATION
#include "text.h"
#include "array.h"
#include "milk.h"
#define WHISPER_IMPLEMENTATION
#include "whisper.h"
#define IMAGINE_IMPLEMENTATION
#include "imagine.h"
#define THREAD_IMPLEMENTATION
#include "thread.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static void test_strings(void) {
  Text t = text_create_ex(3, "12345");
  assert(t.length == 5);

  text_append_str(&t, "hello");
  assert(strcmp(t.data, "12345hello") == 0);
  assert(t.length == 10);

  text_prepend_str(&t, "hello");
  assert(strcmp(t.data, "hello12345hello") == 0);
  assert(t.length == 15);

  text_append(&t, "<%i:%i>", 987, -654);
  assert(strcmp(t.data, "hello12345hello<987:-654>") == 0);
  assert(t.length == 25);

  text_append(&t, "%%%%", 987, 654);
  assert(strcmp(t.data, "hello12345hello<987:-654>%%") == 0);
  assert(t.length == 27);

  text_append(&t, "%f+%f", 5.0055, -12.32);
  assert(strcmp(t.data, "hello12345hello<987:-654>%%5.01+-12.32") == 0);
  assert(t.length == 38);

  text_drop(&t, 12);
  assert(strcmp(t.data, "hello12345hello<987:-654>%") == 0);
  assert(t.length == 26);
}

struct Data {int a,b;};
struct Data create_data(int a, int b) {
  struct Data result;
  result.a = a;
  result.b = b;
  return result;
}

static void test_arrays(void) {
  #define LARGE_VALUE 5107
  {
    double* correct = malloc(LARGE_VALUE * sizeof(*correct));
    double* d = 0;
    int i,j,n=0;
    for (i = 0; i < LARGE_VALUE; ++i) {
      if (n > 0 && !(rand()&5)) {
        /* pop */
        j = rand()%n;
        correct[j] = correct[--n];
        d[j] = d[--array_len_get(d)];
      } else {
        /* push */
        double r = rand() / 7.3;
        correct[n++] = r;
        array_push(d, r);
      }
      assert(n == array_len(d));
      for(j = 0; j < n; ++j)
        assert(correct[j] == d[j]);
    }
    array_free(d);
    free(correct);
  }

  {
    struct Data* correct = malloc(LARGE_VALUE * sizeof(*correct));
    struct Data* d = 0;
    int i,j,a,b,n=0;
    for (i = 0; i < LARGE_VALUE; ++i) {
      if (n > 0 && !(rand()%5)) {
        /* pop */
        a = rand()%n;
        correct[a] = correct[--n];
        d[a] = d[--array_len_get(d)];
      } else {
        /* push */
        a = rand(); b = rand();
        correct[n] = create_data(a,b);
        array_push(d, correct[n]);
        ++n;
      }
      assert(n == array_len(d));
      for(j = 0; j < n; ++j)
        assert(!memcmp(&correct[j], &d[j], sizeof(d[j])));
    }
    array_free(d);
    free(correct);
  }
}

static void test_milk(void) {
  long res1, res2;
  FILE *file;

  res1 = milk_file_get_size_s("src/test.c");
  assert(res1 > 0);

  file = fopen("src/test.c", "rb");
  assert(file);

  res2 = milk_file_get_size_f(file);
  assert(res2 > 0);

  assert(res1 == res2);
  printf("File src/test.c is %li bytes\n", res1);
}

typedef int Job;
struct WorkQueue {
  Job *volatile end;
  Job *volatile next;
  Job jobs[4];
};

#define NUM_ITERS 10000
#define NUM_ITEMS 4
static void thread_adding(void *arg) {
  int volatile *items;
  int i,j;

  items = arg;

  for (i = 0; i < NUM_ITERS; ++i)
  for (j = 0; j < NUM_ITEMS; ++j) {
#if 0
    ++items[j];
#else
    thread_add(&items[j], 1);
#endif
  }

  return;
}

static void thread_workqueue(void *arg) {
  (void)arg;
#if 0
  for (;;) {
    Job job;
    Job *j;
   
    while (work_queue->next == work_queue->end)
      thread_usleep(0, 500);

    j = work_queue->next;
    job = *j;
    if (thread_cas(&work_queue->next, j, j+1) != j)
      continue;

    printf("Performed job %i\n", job);
  }
#endif
  return;
}


static void test_thread(void) {
#define NUM_THREADS 4
  Thread threads[NUM_THREADS];
  int i,err;

  /* test adding */
  {
    int *items;

    items = malloc(sizeof(*items) * NUM_ITEMS);
    for (i = 0; i < NUM_ITEMS; ++i)
      items[i] = 0;

    for (i = 0; i < NUM_THREADS; ++i) {
      err = thread_create(threads+i, thread_adding, items);
      if (err)
        printf("Error creating thread\n"), exit(1);
    }

    printf("Created threads\n");

    for (i = 0; i < NUM_THREADS; ++i) {
      err = thread_join(threads[i]);
      if (err)
        printf("Error while joining threads (%i)\n", err), exit(1);
    }

    printf("Threads joined\n");

    for (i = 0; i < NUM_ITEMS; ++i) {
      printf("%i\n", items[i]);
      if (items[i] != NUM_THREADS*NUM_ITERS)
        printf("Fail test\n"), exit(1);
    }
    free(items);
    for (i = 0; i < NUM_THREADS; ++i)
      thread_free(threads[i]);
  }
  
  /* test workqueue */
  {
    struct WorkQueue work_queue = {0};
    for (i = 0; i < NUM_THREADS; ++i) {
      err = thread_create(threads+i, thread_workqueue, &work_queue);
      if (err)
        printf("Error creating thead\n"), exit(1);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
      err = thread_join(threads[i]);
      if (err)
        printf("Error while joining threads\n"), exit(1);
    }
    for (i = 0; i < NUM_THREADS; ++i) {
      err = thread_free(threads[i]);
      if (err)
        printf("Failed to free thread (%i)\n", err);
    }
  }
}

static void test_whisper_server(void *arg);
static void test_whisper_client(void *arg);

struct Message {int msg_len;};
#define MSG_DECLARE(name, num_bytes) struct Message name[sizeof(int) + (num_bytes+sizeof(struct Message)-1)/sizeof(struct Message)]
#define MSG_STRING(msg) ((char*)((msg)+1))
#define MSG_LEN(msg) (*(int*)(msg))
#define MSG_SIZE(msg) (MSG_LEN(msg) + sizeof(int))
#define SERVER_PORT 7777
static void test_whisper() {
  int i, err;
  unsigned short ports[] = {
    12345,
    12346,
    12347
  };
  Thread threads[ARRAY_LEN(ports)];
  Whisper_TCPServer server;

  err = whisper_init();
  if (err)
    printf("Failed to init whisper\n"), exit(1);

  err = whisper_tcp_server_init(&server, SERVER_PORT);
  if (err)
    printf("Failed to init tcp server\n"), exit(1);
  printf("Put up server on port %i\n", SERVER_PORT);

  /* create some clients */
  for (i = 0; i < ARRAY_LEN(ports); ++i) {
    err = thread_create(threads+i, test_whisper_client, (void*)ports[i]);
    if (err)
      printf("Error while creating thread\n"), exit(1);
  }
  printf("Created %i client threads\n", ARRAY_LEN(ports));

  /* read from clients */

  for (i = 0; i < ARRAY_LEN(ports); ++i) {
    Whisper_TCPConnection conn;
    MSG_DECLARE(msg, 256);
    MSG_DECLARE(expected, 256);
    MSG_DECLARE(received, 256);
    int id;

    id = rand();
    MSG_LEN(msg) = sprintf(MSG_STRING(msg), "Well hello to you! Youre id is %i", id) + 1;
    MSG_LEN(expected) = sprintf(MSG_STRING(expected), "Affirmative, my id is %i", id) + 1;

    err = whisper_tcp_server_poll(server, &conn);
    if (err)
      printf("Server: Failure while polling for connection\n"), exit(1);

    err = whisper_tcp_connection_send(conn, &msg, MSG_SIZE(msg));
    if (err != MSG_SIZE(msg))
      printf("Server: Failed to send '%s' to client\n", MSG_STRING(msg)), exit(1);
    printf("Server: Sent '%s' to client\n", MSG_STRING(msg));

    err = whisper_tcp_connection_receive(conn, (char*)&MSG_LEN(received), sizeof(int));
    if (err != sizeof(int))
      printf("Server: Failed to read size of message from client\n"), exit(1);
    if (MSG_LEN(received) != MSG_LEN(expected))
      printf("Server: Expected message len %i from client, but got %i\n", MSG_LEN(expected), MSG_LEN(received)), exit(1);

    err = whisper_tcp_connection_receive(conn, MSG_STRING(received), MSG_LEN(received));
    if (err != MSG_LEN(received))
      printf("Server: Failed to read %i bytes from connection to %i, only read %i bytes\n", MSG_LEN(received), id, err), exit(1);

    if (strcmp(MSG_STRING(expected), MSG_STRING(received)))
      printf("Server: Expected message %*s, but got %*s\n", MSG_LEN(expected), MSG_STRING(expected), MSG_LEN(received), MSG_STRING(received)), exit(1);

    whisper_tcp_connection_close(conn);
  }

  whisper_tcp_server_close(server);

  for (i = 0; i < ARRAY_LEN(threads); ++i) {
    err = thread_free(threads[i]);
    if (err)
      printf("Failed to free thread\n"), exit(1);
  }
}

static void test_whisper_client(void *arg) {
  Whisper_TCPConnection client;
  int err;
  unsigned short port;
  MSG_DECLARE(msg, 256);
  MSG_DECLARE(received, 256);
  int id;

  port = (unsigned short)arg;

  err = whisper_tcp_client(&client, "localhost", SERVER_PORT);
  if (err)
    printf("Client: Failed to connect to localhost:%i (%i)\n", port, err), exit(1);

  err = whisper_tcp_connection_receive(client, &MSG_LEN(received), sizeof(MSG_LEN(received)));
  if (err != sizeof(MSG_LEN(received)))
    printf("Client: Failed to receive msg length from server (%i)\n", err), exit(1);

  err = whisper_tcp_connection_receive(client, MSG_STRING(received), MSG_LEN(received));
  if (err != MSG_LEN(received))
    printf("Client: Failed to get greeting from server (%i)\n", err), exit(1);
  printf("Client: Got message '%s' from server\n", MSG_STRING(received));

  err = sscanf(MSG_STRING(received), "Well hello to you! Youre id is %i", &id);
  if (err != 1)
    printf("Client: Expected greeting from server, but got '%s'\n", MSG_STRING(received)), exit(1);

  MSG_LEN(msg) = sprintf(MSG_STRING(msg), "Affirmative, my id is %i", id) + 1;

  err = whisper_tcp_connection_send(client, msg, MSG_SIZE(msg));
  if (err != MSG_SIZE(msg))
    printf("Client: Failed to send message to localhost:%i (%i)\n", port, err), exit(1);

  printf("Client: Sent '%s' to server\n", MSG_STRING(msg));
}

int main(int argc, const char *argv[]) {
  (void)argc, (void)argv;
  srand((unsigned int)time(0));
  test_strings();
  test_arrays();
  test_milk();
  test_thread();
  test_whisper();
  return 0;
}
