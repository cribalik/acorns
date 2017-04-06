#define DEBUG 1
#include "text.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

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

int main(int argc, const char *argv[]) {
	(void)argc, (void)argv;

	test_strings();
	return 0;
}
