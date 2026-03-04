#pragma once

#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long int i64;

typedef float f32;
typedef double f64;
typedef bool b8;

#define CPM_U8_MAX 255
#define CPM_I8_MAX 127
#define CPM_U16_MAX 65535
#define CPM_I16_MAX 32767
#define CPM_U32_MAX 4294967295
#define CPM_I32_MAX 2147483647
#define CPM_U64_MAX 18446744073709551615
#define CPM_I64_MAX 9223372036854775807

#define CPM_F32_MAX 3.402823466e+38f
#define CPM_F64_MAX 1.79769e+308

#define Bit(n) ((n) / 8.0f)
#define KB(n) ((n) / 1000.0f)
#define MB(n) ((n) / 1000000.0f)
#define GB(n) ((n) / 1000000000.0f)

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)
