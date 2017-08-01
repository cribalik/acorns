#include <stdio.h>

static long milk_file_get_size_f(FILE *file) {
	long result, old_pos;

	old_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	result = ftell(file);
	fseek(file, old_pos, SEEK_SET);
	return result;
}

static long milk_file_get_size_s(const char *filename) {
	int err;
	long result;
	FILE *file;

	file = fopen(filename, "rb");
	if (!file) return -1;

	err = fseek(file, 0, SEEK_END);
	if (err) {
		fclose(file);
		return -1;
	}

	result = ftell(file);
	fclose(file);

	return result;
}
