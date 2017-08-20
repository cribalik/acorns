#ifndef UTILS_H
#define UTILS_H

#define STATIC_ASSERT(expr, name) typedef char static_assert_##name[expr?1:-1]
#define ALIGN(x, val) ALIGN_MASK(x, val-1)
#define ALIGN_MASK(x, mask) (((x)+(mask)) & ~(mask))
#define ARRAY_LEN(a) ((int)(sizeof(a)/sizeof(*a)))

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
	typedef char i8;
	typedef short i16;
	typedef int i32;
	typedef long i64;
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long u64;
#endif

STATIC_ASSERT(sizeof(i8) == 1, char_is_1_byte);
STATIC_ASSERT(sizeof(i16) == 2, char_is_2_bytes);
STATIC_ASSERT(sizeof(i32) == 4, char_is_4_bytes);
STATIC_ASSERT(sizeof(i64) == 8, char_is_8_bytes);

#endif
