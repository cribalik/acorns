#ifndef MILK_H
#define MILK_H

#include <stdio.h>

#ifndef OS_DEFINED
	#ifdef _MSC_VER
		#define OS_WINDOWS 1
	#elif defined(__linux__)
		#define OS_LINUX 1
	#else
		#error "Unimplemented platform"
	#endif
#endif /*OS_DEFINED*/

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

	#ifdef OS_WINDOWS
	if (fopen_s(&f, filename, "rb"))
		f = 0;
	#else
	f = fopen(filename, "rb");
	#endif /*OS*/

	if (!f)
		return -1;

	size = milk_file_get_size_f(f);
	data = (unsigned char*)malloc(*size_out);
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

#ifdef OS_WINDOWS
	if (fopen_s(&file, filename, "rb"))
		file = 0;
#else
	file = fopen(filename, "rb");
#endif /*OS*/

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