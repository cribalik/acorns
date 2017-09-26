#ifndef MILK_H
#define MILK_H

#include <stdio.h>

#ifndef MILK_NO_STATIC
  #define MILK__CALL static
#else
  #define MILK__CALL
#endif

MILK__CALL long milk_file_get_size_f(FILE *file);
MILK__CALL int milk_file_get_contents(const char *filename, unsigned char **data_out, long *size_out);
MILK__CALL long milk_file_get_size_s(const char *filename);

#endif

#ifdef MILK_IMPLEMENTATION

MILK__CALL int milk_file_get_contents(const char *filename, unsigned char **data_out, long *size_out) {
	FILE *f;
	long size, num_read;
	unsigned char *data;

	f = fopen(filename, "rb");
	if (!f)
		return -1;

	size = milk_file_get_size_f(f);
	data = malloc(*size_out);
	num_read = fread(data, 1, size, f);
	if (num_read != size)
		goto err;

	fclose(f);
	*size_out = size;
	*data_out = data;

	return 0;

	err:
	fclose(f);
	return -1;
}

MILK__CALL long milk_file_get_size_f(FILE *file) {
	long result, old_pos;

	old_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	result = ftell(file);
	fseek(file, old_pos, SEEK_SET);
	return result;
}

MILK__CALL long milk_file_get_size_s(const char *filename) {
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

#endif