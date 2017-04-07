#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "text.h"
#include "text_alloc.h"

#ifndef TEXT_INITIAL_SIZE
	#define TEXT_INITIAL_SIZE 8
#endif

static char text_null = '\0';

#define TEXT_IS_ALLOCATED(s) (((s).data) != &text_null)

static void text_reserve(Text* a) {
	if (!TEXT_IS_ALLOCATED(*a)) {
		a->data = TEXT_MALLOC(a->length+1);
		a->capacity = a->length+1;
	}
	else if (a->length+1 > a->capacity) {
		while (a->capacity < a->length) a->capacity *= 2;
		a->data = TEXT_REALLOC(a->data, a->capacity);
	}
}

int text_append_str(Text* a, const char* b) {
	int old_len = a->length;
	a->length += strlen(b);
	text_reserve(a);
	memcpy(a->data + old_len, b, a->length - old_len);
	a->data[a->length] = 0;
	return a->length - old_len;
}

Text text_create() {
	Text result = {0};
	result.data = &text_null;
	return result;
}

Text text_create_ex(int capacity, char* initial_value) {
	Text result = {0};
	assert(capacity >= 0);

	result.capacity = capacity;
	if (capacity) {
		result.data = TEXT_MALLOC(capacity);
	} else {
		result.data = &text_null;
	}

	if (initial_value) {
		text_append_str(&result, initial_value);
	}

	return result;
}

int text_prepend_str(Text* a, char* b) {
	int old_len = a->length;
	a->length += strlen(b);
	text_reserve(a);
	memmove(a->data + a->length - old_len, a->data, old_len);
	memcpy(a->data, b, a->length - old_len);
	a->data[a->length] = 0;
	return a->length - old_len;
}

void text_drop(Text* s, int n) {
	s->length -= n;
	s->data[s->length] = 0;
}

void text_clear(Text* s) {
	s->length = 0;
	*s->data = 0;
}

void text_free(Text s) {
	TEXT_FREE((void*) s.data);
}

void text_append_char(Text* s, char c) {
	++s->length;
	text_reserve(s);
	s->data[s->length-1] = c;
	s->data[s->length] = 0;
}

void text_append_int(Text* s, int i) {
	text_append_long(s, (long) i);
}

void text_append_long(Text* s, long i) {
	char buf[32];
	char* b = buf + 30;
	int neg = i < 0;
	b[31] = 0;
	if (neg) i *= -1;
	while (i) {
		*b-- = '0' + i%10;
		i /= 10;
	}
	if (neg) *b-- = '-';
	text_append_str(s, b+1);
}

void text_append_double(Text* s, double d) {
	int i;
	if (d < 0) {
		text_append_char(s, '-');
		d *= -1;
	}
	text_append_long(s, (long)d);
	d -= (double)(long)d;
	text_append_char(s, '.');
	for (i = 0; i < 2; ++i) {
		d *= 10.0;
		text_append_char(s, '0' + ((int)(d+0.5))%10);
	}
}

void text_append(Text* s, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			text_append_char(s, *fmt);
		} else {
			++fmt;
			switch (*fmt) {

				case 'i': text_append_int(s, va_arg(args, int)); break;

				case 's': text_append_str(s, va_arg(args, char*)); break;

				case '%': text_append_char(s, '%'); break;

				case 'f': text_append_double(s, va_arg(args, double)); break;

			}
		}
	}
	va_end(args);
}
