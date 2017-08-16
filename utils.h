#ifndef UTILS_H
#define UTILS_H

#define STATIC_ASSERT(expr, name) typedef char static_assert_##name[expr?1:-1];
#define ALIGN(x, val) ALIGN_MASK(x, val-1)
#define ALIGN_MASK(x, mask) (((x)+(mask)) & ~(mask))

typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

STATIC_ASSERT(sizeof(i8) == 1, char_is_1_byte);
STATIC_ASSERT(sizeof(i16) == 2, char_is_2_bytes);
STATIC_ASSERT(sizeof(i32) == 4, char_is_4_bytes);
STATIC_ASSERT(sizeof(i64) == 8, char_is_8_bytes);

#endif