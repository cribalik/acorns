#ifndef STRING_H
#define STRING_H

typedef struct Text {
	char* data;
	int length, capacity;
} Text;

Text text_create();
Text text_create_ex(int capacity, char* initial_value);

void text_append(Text* s, const char* fmt, ...);
int  text_append_str(Text* a, const char* b);
void text_append_char(Text* s, char c);
void text_append_int(Text* s, int c);
void text_append_long(Text* s, long c);
void text_append_double(Text* s, double c);

int text_prepend_str(Text* a, char* b);

void text_drop(Text* s, int n);
void text_clear(Text* s);

void text_free(Text s);

#endif
