#pragma once

#include <sys/types.h>

typedef	__int8				int8;
typedef	__int16				int16;
typedef	__int32				int32;
typedef	__int64				int64;

typedef	unsigned __int8		uint8;
typedef	unsigned __int16	uint16;
typedef	unsigned __int32	uint32;
typedef	unsigned __int64	uint64;

typedef	unsigned char		byte;
typedef	wchar_t				wchar;
typedef unsigned short		ushort;
typedef unsigned long		ulong;
typedef int64				size;

typedef	uint32				id32;
typedef	uint64				id64;

typedef int32				tid;

#ifndef _STDINT
#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64
#endif _STDINT
