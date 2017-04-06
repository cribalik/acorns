#ifndef BELT_TYPES_H
#define BELT_TYPES_H

typedef unsigned int uint;

/* 8 bits */
typedef unsigned char byte;
typedef unsigned char u8;
typedef char i8;

/* 16 bits */
typedef short i16;
typedef unsigned short u16;

/* 32 bits */
typedef int i32;
typedef unsigned int u32;

/* 64 bits */
typedef long i64;
typedef unsigned long u64;

/* floats */
typedef float f32;
typedef double f64;

/* max and min */
#define I8_MAX    0x7F
#define I8_MIN   -128
#define I16_MAX   0x7FFF
#define I16_MIN  -32768
#define I32_MAX   0x7FFFFFFF
#define I32_MIN  -2147483648
#define I64_MAX   9223372036854775807
#define I64_MIN  -9223372036854775808

#define U8_MAX    0xFF
#define U16_MAX   0xFFFF
#define U32_MAX   0xFFFFFFFF
#define U64_MAX   18446744073709551615

#define F32_MAX   3.402823466e+38f
#define F64_MAX   1.7976931348623158e+308

#endif
