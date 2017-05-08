#define DEBUG 1
#include "text/text.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "array/array.h"

void test_strings() {
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

void test_arrays() {
  #define LARGE_VALUE 5107
  {
    double* correct = malloc(LARGE_VALUE * sizeof(*correct));
    double* d = 0;
    int i,j,n=0;
    srand(time(0));
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
    srand(time(0));
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

int main(int argc, const char *argv[]) {
  (void)argc, (void)argv;
  test_strings();
  test_arrays();
  return 0;
}
