#ifndef UTILS_H
#define UTILS_H

#ifdef _MSC_VER
	#define OS_WINDOWS 1
#elif defined(__linux__)
	#define OS_LINUX 1
#else
	#error "Unimplemented platform"
#endif

#include <stddef.h>

#define STATIC_ASSERT(expr, name) typedef char static_assert_##name[expr?1:-1]
#define align(ptr, n) (void*)(((long)(ptr)+((n)-1)) & ~((n)-1))
#define containerof(ptr, type, member) (((type)*)((char*)ptr - offsetof(type, member)))
#ifdef OS_WINDOWS
	#define alignof __alignof
#else
	#define alignof(type) offsetof(struct {char a; type b;}, b)
#endif
#define ARRAY_LEN(a) ((int)(sizeof(a)/sizeof(*a)))

#define Kilobyte(n) ((n)*1024L)
#define Megabyte(n) (Kilobyte(n)*1024L)
#define Gigabyte(n) (Megabyte(n)*1024L)

static unsigned int random(unsigned int *r) {
  return *r = 1103515245 * *r + 12345;
}

#ifdef _MSC_VER
	typedef __int8 i8;
	typedef __int16 i16;
	typedef __int32 i32;
	typedef __int64 i64;
	typedef unsigned __int8 u8;
	typedef unsigned __int16 u16;
	typedef unsigned __int32 u32;
	typedef unsigned __int64 u64;
#else
	/* let's hope stdint has us covered */
	#include <stdint.h>
	typedef int8_t i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
#endif

typedef union {i64 a; u64 b; double c;} aligned_t;


STATIC_ASSERT(sizeof(i8) == 1, i8_is_1_byte);
STATIC_ASSERT(sizeof(i16) == 2, i16_is_2_bytes);
STATIC_ASSERT(sizeof(i32) == 4, i32_is_4_bytes);
STATIC_ASSERT(sizeof(i64) == 8, i64_is_8_bytes);
STATIC_ASSERT(sizeof(u8) == 1, u8_is_1_byte);
STATIC_ASSERT(sizeof(u16) == 2, u16_is_2_bytes);
STATIC_ASSERT(sizeof(u32) == 4, u32_is_4_bytes);
STATIC_ASSERT(sizeof(u64) == 8, u64_is_8_bytes);


#endif
