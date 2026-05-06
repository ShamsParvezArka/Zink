#ifndef BASE_H
#define BASE_H

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

////////////////////////////////
// NOTE: Codebase Keyword
//
#define global   static
#define local    static
#define internal static

////////////////////////////////
// NOTE: Basic Constants
//
#define PI     3.14159265358979324f
#define E      2.71828182845904523f
#define LOG2E  1.44269504088896339f
#define LOG10E 0.43429448190325182f
#define DegToRad(x) (f32)(((x) * PI) / 180)

////////////////////////////////
// NOTE: Units
//
#define KB(n)  (((u64)(n)) << 10)
#define MB(n)  (((u64)(n)) << 20)
#define GB(n)  (((u64)(n)) << 30)
#define TB(n)  (((u64)(n)) << 40)

////////////////////////////////
// NOTE: Helper Macros
//
#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(a, b) a##b
#define Glue(a, b) Glue_(a, b)

////////////////////////////////
// NOTE: Clamp, Min, Max
//
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define Swap(type, a, b) do {type Glue(_type_, __LINE__) = a; a = b; b = Glue(_type_, __LINE__);} while (0)

////////////////////////////////
// NOTE: Memory Operation
//
#if defined(_MSC_VER)
#  define alignof(type) __alignof(type)
#endif
#define AlignPow2(ptr, x) (((ptr) + (x) - 1) & (~((x) - 1)))
#define IsPow2(x)         ((x) != 0 && ((x) & ((x) - 1)) == 0)

#define MemoryCompare(a, b, size) memcmp((a), (b), (size))
#define MemoryMatch(a, b, size)   (MemoryCompare((a), (b), (size)) == 0)
#define MemoryZero(s, size)       memset((s), 0, (size))
#define MemoryCstrLength(s)       strlen((s))
#define MemoryCopy(dst, src, s)   memcpy((dst), (src), s)

////////////////////////////////
// NOTE: Loop macros
//
#define DeferScope(begin, end)        for (u32 Glue(_i_, __LINE__) = ((begin), 0); !Glue(_i_, __LINE__); Glue(_i_, __LINE__) += 1, (end))
#define DeferScopeChecked(begin, end) for (u32 Glue(_i_, __LINE__) = 2 * !(begin); (Glue(_i_, __LINE__) == 2 ? ((end), 0) : !Glue(_i_, __LINE__)); Glue(_i_, __LINE__) += 1, (end))

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define ForEach(type_var, array)                                        \
  for (u64 Glue(_i_, __LINE__) = 0, Glue(_s_, __LINE__) = CArrayCount(array); \
       Glue(_i_, __LINE__) < Glue(_s_, __LINE__);                       \
       Glue(_i_, __LINE__)++)                                           \
    for (u64 _done_ = 0; !_done_; _done_ += 1)                          \
      for (type_var = (array)[Glue(_i_, __LINE__)]; !_done_; _done_ += 1)

////////////////////////////////
// NOTE: Base Types
//
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;
typedef i32      b32;

////////////////////////////////
// NOTE: Useful structs
//
typedef struct Range Range;
struct Range
{
  i64 min;
  i64 max;
};

typedef struct DateTime DateTime;
struct DateTime
{
  u16 micro_sec;
  u16 milli_sec;
  u16 sec;
  u16 min;
  u16 hour;
  u16 day;
  u16 mon;
  u32 year;
};

global const u8 integer_symbol_reverse[128] =
{
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};


internal void Win32SpawnConsole();
internal DateTime DateTimeFromUnixTime(u64 time);

#endif // BASE_H
