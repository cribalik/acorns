#include <stdlib.h>

#define ARRAY_INITIAL_SIZE 4

struct ArrayHeader {int length, capacity;};

static struct ArrayHeader* array_header(void* arr) {
  return (struct ArrayHeader*) ((char*)arr - sizeof(struct ArrayHeader));
}

void* array_internal_create(int item_size) {
  void* result = (char*)malloc(ARRAY_INITIAL_SIZE * item_size + sizeof(struct ArrayHeader)) + sizeof(struct ArrayHeader);
  array_header(result)->capacity = ARRAY_INITIAL_SIZE;
  return result;
}

void array_internal_push(void** arrp, int size) {
  struct ArrayHeader* h = array_header(*arrp);
  if (h->length == h->capacity) {
    array_header(*arrp)->capacity *= 2;
    *arrp = ((char*) realloc(h, sizeof(struct ArrayHeader) + h->capacity*size) + sizeof(struct ArrayHeader));
    h = array_header(*arrp);
  }
  ++h->length;
}

int array_len(void* a) {
  return array_header(a)->length;
}

int array_capacity(void* a) {
  return array_header(a)->capacity;
}
