#if 0
#define _XOPEN_SOURCE 500
#else
#define _POSIX_C_SOURCE 201112L
#endif

#define DEBUG 1

#define MEM_IMPLEMENTATION
#include "mem.h"
#define WORK_QUEUE_IMPLEMENTATION
#include "work_queue.h"
#include "utils.h"
#define TEXT_IMPLEMENTATION
#include "text.h"
#include "array.h"
#define MILK_IMPLEMENTATION
#define MILK_NO_STATIC
#include "milk.h"
#define WHISPER_IMPLEMENTATION
#define WHISPER_NO_STATIC
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

  printf("Testing string.. ");
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
  printf("Success\n");
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
  printf("Testing array of doubles.. ");
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
  printf("Success\n");

  printf("Testing array of structs.. ");
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
  printf("Success\n");
}

static void test_milk(void) {
  long res1, res2;
  FILE *file;

  printf("Testing milk get file size.. ");

  res1 = milk_file_get_size_s("src/test.c");
  assert(res1 > 0);

  file = fopen("src/test.c", "rb");
  assert(file);

  res2 = milk_file_get_size_f(file);
  assert(res2 > 0);

  assert(res1 == res2);
  printf("Success\n");
}

#define NUM_ITEMS 10000
#define NUM_ITERS 1000
static void thread_adding(void *data) {
  AtomicInt *items = (AtomicInt*)data;
  int i,j;

  for (j = 0; j < NUM_ITERS; ++j) {
    for (i = 0; i < NUM_ITEMS; ++i) {
      #if 1
      atomic_add_int(items+i, 1);
      #else
      ++items[i].val;
      #endif
    }
  }
}

static void thread_test_spinlock(void *data);
static void test_workqueue(void);

typedef struct {
  SpinLock *lock;
  AtomicInt *value;
  int id;
  int num_iters;
} SpinLockTestData;

#define NUM_THREADS 16

static void test_atomic_add() {
  int *items;
  int i;
  Thread threads[NUM_THREADS];

  printf("Testing atomic_add_int.. ");
  fflush(stdout);

  items = malloc(sizeof(*items) * NUM_ITEMS);
  for (i = 0; i < NUM_ITEMS; ++i)
    items[i] = 0;

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_create(threads+i, thread_adding, items))
      printf("Error creating thread\n"), exit(1);

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_join(threads+i))
      printf("Error while joining threads\n"), exit(1);

  for (i = 0; i < NUM_ITEMS; ++i)
    if (items[i] != NUM_THREADS*NUM_ITERS)
      printf("failed test. Sum should be %i but was %i\n", NUM_THREADS*NUM_ITERS, items[i]), exit(1);
  free(items);

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_free(threads+i))
      printf("Failed to free thread\n"), exit(1);

  printf("Success\n");
}

static void test_spinlock() {
  const int num_iters = 1000;
  Thread threads[NUM_THREADS];
  int i;
  SpinLock lock;
  AtomicInt value;
  SpinLockTestData data[NUM_THREADS];

  printf("Testing spinlock.. "), fflush(stdout);

  spinlock_init(&lock);
  value.val = 0;

  for (i = 0; i < NUM_THREADS; ++i)
    data[i].lock = &lock,
    data[i].value = &value,
    data[i].id = i,
    data[i].num_iters = num_iters;

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_create(threads+i, thread_test_spinlock, &data))
      printf("Error creating thread\n"), exit(1);

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_join(threads+i))
      printf("Error while joining threads\n"), exit(1);

  if (value.val != NUM_THREADS * num_iters)
    printf("Failed spinlock test, expected %i but got %i\n", NUM_THREADS*num_iters, value.val), exit(1);

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_free(threads+i))
      printf("Failed to free thread\n"), exit(1);

  printf("Success\n");
}

static void test_thread(void) {
  test_atomic_add();
  test_spinlock();

  #if 1
  test_workqueue();
  #endif
}

static void thread_test_spinlock(void *data) {
  SpinLockTestData *in;
  int newval, i;

  in = (SpinLockTestData*)data;

  for (i = 0; i < in->num_iters; ++i) {
    spinlock_lock(in->lock);

    newval = atomic_read_int(in->value) + 1;
    atomic_write_int(in->value, newval);

    spinlock_unlock(in->lock);
  }
}

static void thread_work(void *data) {
  *(int*)data += 1;
}

static void thread_worker(void *data) {
  WorkQueue *queue;
  Job job;

  queue = (WorkQueue*)data;

  for (;;) {
    work_queue_pop(queue, &job);
    job.fun(job.data);
    work_queue_job_done(queue);
  }
}

static void test_workqueue(void) {
  #define NUM_JOBS 1000

  int i;
  Thread threads[NUM_THREADS];
  int data[NUM_JOBS];
  WorkQueue queue = {0};

  printf("Testing workqueue.. "), fflush(stdout);

  for (i = 0; i < NUM_JOBS; ++i)
    data[i] = 0;

  work_queue_init(&queue, 32);

  for (i = 0; i < NUM_THREADS; ++i)
    if (thread_create(threads+i, thread_worker, &queue))
      printf("Failed to create thread\n"), exit(1);

  for (i = 0; i < NUM_JOBS; ++i)
    while (work_queue_push(&queue, thread_work, data+i));

  while (!work_queue_isdone(&queue, NUM_JOBS));

  for (i = 0; i < NUM_JOBS; ++i)
    if (data[i] > 1)
      printf("Failed test, job number %i was executed %i times\n", i, data[i]), exit(1);
    else if (data[i] < 1)
      printf("Failed test, job number %i was skipped\n", i), exit(1);

  printf("Success\n");
  #undef NUM_JOBS
}

static void test_whisper_client(void *arg);

struct Message {int msg_len;};
#define MSG_DECLARE(name, num_bytes) struct Message name[sizeof(int) + (num_bytes+sizeof(struct Message)-1)/sizeof(struct Message)]
#define MSG_STRING(msg) ((char*)((msg)+1))
#define MSG_LEN(msg) (*(int*)(msg))
#define MSG_SIZE(msg) ((int)(MSG_LEN(msg) + sizeof(int)))
#define SERVER_PORT 7777
static void test_whisper(void) {
  int i, err;
  unsigned short ports[] = {
    12345,
    12346,
    12347
  };
  Thread threads[ARRAY_LEN(ports)];
  Whisper_TCPServer server;

  printf("Testing whisper.."), fflush(stdout);

  err = whisper_init();
  if (err)
    printf("Failed to init whisper\n"), exit(1);

  err = whisper_tcp_server_init(&server, SERVER_PORT);
  if (err)
    printf("Failed to init tcp server\n"), exit(1);
  printf("Put up server on port %i\n", SERVER_PORT);

  /* create some clients */
  for (i = 0; i < ARRAY_LEN(ports); ++i) {
    err = thread_create(threads+i, test_whisper_client, (void*)(long)ports[i]);
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
      printf("Server: Expected message %.*s, but got %.*s\n", MSG_LEN(expected), MSG_STRING(expected), MSG_LEN(received), MSG_STRING(received)), exit(1);

    whisper_tcp_connection_close(conn);
  }

  whisper_tcp_server_close(server);

  for (i = 0; i < ARRAY_LEN(threads); ++i) {
    err = thread_free(threads+i);
    if (err)
      printf("Failed to free thread\n"), exit(1);
  }

  err = whisper_close();
  if (err)
    printf("Failed to close whisper\n"), exit(1);

  printf("Success\n");
}

static void test_whisper_client(void *arg) {
  Whisper_TCPConnection client;
  int err;
  unsigned short port;
  MSG_DECLARE(msg, 256);
  MSG_DECLARE(received, 256);
  int id;

  port = (unsigned short)(long)arg;

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

  err = whisper_tcp_connection_close(client);
  if (err)
    printf("Failed to close client connection\n"), exit(1);

  printf("Client: Sent '%s' to server\n", MSG_STRING(msg));
}

static void test_utils(void) {
  printf("Testing utils.. "), fflush(stdout);

  assert(alignof(int) == sizeof(int));
  assert(alignof(struct {char a; i16 b;}) == 2);
  assert(alignof(struct {char a; i32 b;}) == 4);
  assert(alignof(struct {char a; i64 b;}) == 8);
  assert(alignof(struct {char a; double b;}) == sizeof(double));
  assert( align((void*)0, 4) == (void*)0 );
  assert( align((void*)1, 4) == (void*)4 );
  assert( align((void*)2, 4) == (void*)4 );
  assert( align((void*)3, 4) == (void*)4 );
  assert( align((void*)4, 4) == (void*)4 );
  assert( align((void*)5, 4) == (void*)8 );
  assert( align((void*)6, 4) == (void*)8 );
  assert( align((void*)7, 4) == (void*)8 );
  assert( align((void*)8, 4) == (void*)8 );

  printf("Success\n");
}

static void test_stack_allocator() {
  Stack stack;
  unsigned char *data;
  int size;

  printf("Testing stack allocator.. "), fflush(stdout);

  size = 19;
  data = malloc(size);
  if (!data)
    printf("Failed to alloc\n"), exit(1);

  stack_init(&stack, data, size);
  assert(stack_push(&stack, i8) == data);
  assert(stack.curr == data+1);
  assert(stack_push(&stack, i16) == data+2);
  assert(stack.curr == data+4);
  assert(stack_push(&stack, i64) == data+8);
  assert(stack.curr == data+16);
  assert(!stack_push(&stack, i32) && mem_errno == MEM_FULL);
  assert(stack.curr == data+16);
  assert(!stack_push(&stack, i64) && mem_errno == MEM_FULL);
  assert(stack.curr == data+16);
  assert(stack_push(&stack, i16) == data+16);
  assert(stack.curr == data+18);
  assert(!stack_push(&stack, i16) && mem_errno == MEM_FULL);
  assert(stack.curr == data+18);
  assert(stack_push(&stack, i8) == data+18);
  assert(stack.curr == data+19 && stack.curr == stack.end);

  stack_pop(&stack, data+13);
  assert(stack.curr == data+13);

  stack_clear(&stack);
  assert(stack.curr == data && stack.curr == stack.begin);

  printf("Success\n");
}

static void test_block_allocator() {
  Block block;
  typedef struct Data {int a,b,c;} Data;
  Data *data;
  int size;

  printf("Testing block allocator.. "), fflush(stdout);

  size = 5;
  data = malloc(size);
  if (!data)
    printf("Failed to alloc\n"), exit(1);

  assert(block_init(&block, data, 5, 3) == MEM_INVALID_ARG && mem_errno == MEM_INVALID_ARG);
  assert(block_init(&block, (void*)1, 5, sizeof(void*)) == MEM_INVALID_ALIGN && mem_errno == MEM_INVALID_ALIGN);

  assert(block_init(&block, data, size, sizeof(Data)) == 0);
  assert(block_get(&block) == (void*)(data));
  assert(block_get(&block) == (void*)(data+1));
  assert(block_get(&block) == (void*)(data+2));
  assert(block_get(&block) == (void*)(data+3));
  assert(block_get(&block) == (void*)(data+4));
  assert(!block_get(&block) && mem_errno == MEM_FULL);
  assert(!block_get(&block) && mem_errno == MEM_FULL);
  block_put(&block, data+1);
  assert(block_get(&block) == (void*)(data+1));

  printf("Success\n");
}

int main(int argc, const char *argv[]) {
  (void)argc, (void)argv;
  srand((unsigned int)time(0));
  test_strings();
  test_arrays();
  test_milk();
  test_thread();
  test_whisper();
  test_utils();
  test_stack_allocator();
  test_block_allocator();
  return 0;
}
