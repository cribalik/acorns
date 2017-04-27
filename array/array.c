#include <stdlib.h>
#include <stddef.h>

#define ARRAY_INITIAL_SIZE 4

struct ArrayHeader {int length, capacity; union {size_t s; void* v; long l; long double d;} data;};

static struct ArrayHeader* array_header(void* arr) {
  return (struct ArrayHeader*) ((char*)arr - offsetof(struct ArrayHeader, data));
}

void* array_internal_create(int item_size) {
  struct ArrayHeader* h;
  h = malloc(offsetof(struct ArrayHeader, data) +ARRAY_INITIAL_SIZE * item_size);
  h->length = 0;
  h->capacity = ARRAY_INITIAL_SIZE;
  return &h->data;
}

void array_internal_push(void** arrp, int size) {
  struct ArrayHeader* h = array_header(*arrp);
  if (h->length++ == h->capacity) {
    h->capacity *= 2;
    *arrp = &((struct ArrayHeader*)realloc(h, offsetof(struct ArrayHeader, data) + h->capacity*size))->data;
  }
}

int* array_internal_len(void* a) {
  return &array_header(a)->length;
}

void array_free(void* a) {
  free(array_header(a));
}
