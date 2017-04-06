typedef struct String {
  int lenth, capacity;
  char* chars;
} String;

int    string_append(String* a, const char* b);
String string_create(char* initial_value);
int    string_prepend(String* a, char* b);
void   string_pop(String* s, int n);
char*  string_get(String s);
char*  string_clear(String* s);
void   string_free(String* s);
void   string_append_char(String* s, char c);
