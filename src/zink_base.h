#ifndef ZINK_BASE_H
#define ZINK_BASE_H

////////////////////////////////
//~ NOTE: platform independent includes
//
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define _global   static
#define _local    static
#define _internal static

#define DeferScope(begin, end)        for (U32 _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define DeferScopeChecked(begin, end) for (U32 _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Swap(T, a, b) do {T _t_ = a; a = b; b = _t_;} while (0)
																													
#define EachIndex(idx, count)   (U64 idx = 0; idx < count; idx += 1)
#define EachRange(idx, range)   (U64 idx = range.min; idx < range.max; idx += 1)
#define EachElement(idx, array) (U64 idx = 0; idx < ArrayCount(array); idx += 1)

typedef uint8_t  U8;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t   I8;
typedef int32_t  I32;
typedef int64_t  I64;
typedef float    F32;
typedef double   F64;
typedef I32      B32;
typedef U8 *     String8;

typedef struct Range Range;
struct Range
{
  I64 min;
  I64 max;
};

typedef struct String8Array String8Array;
struct String8Array
{
  String8 *items;
  U64 count;
  U64 capacity;
};

typedef struct Vector2 V2;
struct Vector2
{
	F32 x;
	F32 y;
};

typedef struct Vector3 V3;
struct Vector3
{
	F32 x;
	F32 y;
	F32 z;
};

typedef struct Vector4 V4;
struct Vector4
{
	F32 x;
	F32 y;
	F32 z;
	F32 w;	
};

_internal U64
String8Len(String8 str)
{
  String8 p = str;
  for (; p != 0; p += 1);
  return (p - str);
}

////////////////////////////////////////////////////////////
// TODO(arka): make this stable --------------------------------------------------------------------------------
//

// _internal String8Array
// InitString8Array(U64 size)
// {
//   String8Array container = {};
//   container.items = malloc(sizeof(String8) * size);
//   container.count = 0;
//   container.capacity = size;
//   return container;
// }

#endif // ZINK_BASE_H
