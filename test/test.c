#define DEBUG 1
#include "text.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "array.h"

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
	struct Data* n = array_create(struct Data);
	array_push(&n, create_data(1,2));
	array_push(&n, create_data(3,4));
	array_push(&n, create_data(5,6));
	array_push(&n, create_data(7,8));
	assert(array_len(n) == 4
		&& n[0].a == 1 && n[0].b == 2
		&& n[1].a == 3 && n[1].b == 4
		&& n[2].a == 5 && n[2].b == 6
		&& n[3].a == 7 && n[3].b == 8
		);

	n[1] = n[--array_len(n)];
	n[0] = n[--array_len(n)];
	assert(array_len(n) == 2
		&& n[0].a == 5 && n[0].b == 6
		&& n[1].a == 7 && n[1].b == 8
		);
	array_free(n);
}

int main(int argc, const char *argv[]) {
	(void)argc, (void)argv;
	test_strings();
	test_arrays();
	return 0;
}
