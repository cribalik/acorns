#include "array.c"
#include <assert.h>
#include <string.h>

#ifndef STRING_INITIAL_SIZE
  #define STRING_INITIAL_SIZE 8
#endif

static char string_null = '\0';

#define STRING_IS_ALLOCED(s) ((s.chars) != &string_null)

int string_append(String* a, const char* b) {
  int len;
  char* next;

  a->length += strlen(b);

  /* alloc */
  if (!STRING_IS_ALLOCED(*a)) {
    a->chars = malloc(len);
    a->capacity = len;
  }
  else if (a->length+1 > a->capacity) {
    while (a->capacity < a->length) a->capacity *= 2;
    a->chars = realloc(a->chars, a->capacity);
  }

  memcpy(a->chars + a->length - len - 1, b, len);
  a->chars[a->length] = 0;
  return len;
}

String string_create() {
  String result = {0};
  result.chars = &string_null;
  return result;
}

String string_create_ex(int capacity, char* initial_value) {
  String result = {0};
  assert(capacity >= 0);
  if (capacity) {
    result.chars = malloc(capacity);
  } else {
    result.chars = &string_null;
  }

  if (initial_value) {
    string_append(&result, initial_value);
  }

  return result;
}

int string_prepend(String* a, char* b) {
  int len, old_len;
  char* next;
  len = strlen(b);
  old_len = a->chars.count;
  /* move current string to end */
  next = array_pushN(&a->chars, len);
  next -= 2;
  memmove(next, a->chars.data, old_len);

  /* prepend new string */
  next = a->chars.data;
  while (*b) {
    *next = *b;
    ++next, ++b;
  }
  return len;
}

void string_pop(String* s, int n) {
  assert(s->chars.count > n);
  array_pop_N(&s->chars, n);
  *(char*)array_last(&s->chars) = 0;
}

char* string_get(String* s) {
  return array_begin(&s->chars);
}

char* string_clear(String* s) {
  s->chars.count = 1;
  *(char*)array_last(&s->chars) = 0;
  return s->chars.data;
}

void string_free(String* s) {
  array_free(&s->chars);
}

void string_append_char(String* s, char c) {
  char* p = array_push(&s->chars);
  p[-1] = c;
  *p = 0;
}
