#if 0
#define _XOPEN_SOURCE 500
#else
#define _POSIX_C_SOURCE 199309L
#endif

#define DEBUG 1
#define TEXT_IMPLEMENTATION
#include "text/text.h"
#include "array/array.h"
#include "milk/milk.h"
#define THREAD_IMPLEMENTATION
#include "thread/thread.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static void test_strings() {
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

static void test_arrays() {
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

static void test_milk() {
  long res1, res2;
  FILE *file;

  res1 = milk_file_get_size_s("test/test.c");
  assert(res1 > 0);

  file = fopen("test/test.c", "rb");
  assert(file);

  res2 = milk_file_get_size_f(file);
  assert(res2 > 0);

  assert(res1 == res2);
  printf("File test/test.c is %li bytes\n", res1);
}

static THREAD_PROC(do_something, arg) {
  int ms = 300 + (rand()%401);
  thread_usleep(1, ms);
  printf("Thread %li slept for %i ms\n", (long)arg, 1000+ms);
  return 0;
}

static void test_thread() {
  Thread threads[4];
  int i, err;

  for (i = 0; i < 4; ++i) {
    err = thread_create(threads+i, do_something, (void*)(long)i);
    if (err)
      printf("Error creating thread\n"), exit(1);
  }

  for (i = 0; i < 4; ++i) {
    err = thread_join(threads[i]);
    if (err)
      printf("Error while joining threads\n"), exit(1);
  }
}

int main(int argc, const char *argv[]) {
  (void)argc, (void)argv;
  srand(time(0));
  test_strings();
  test_arrays();
  test_milk();
  test_thread();
  return 0;
}
