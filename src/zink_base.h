#ifndef ZINK_BASE_H
#define ZINK_BASE_H

#include <stdint.h>
#include <stdbool.h>

#define _global   static
#define _local    static
#define _internal static

#define DeferScope(begin, end)       for (U32 _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define DeferScopeChecked(begin, end) for (int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

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

_internal U64
String8Len(String8 str)
{
  String8 p = str;
  for (; p != 0; p += 1);
  return (p - str);
}

#endif // ZINK_BASE_H
