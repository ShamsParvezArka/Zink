#define StringLit(s) StringFromCstr((s))
#define StringArg(s) (i32)(s).count, (s).value

#define StringBuilderPush(sb, n) DynArrayPush(sb)
#define StringBuilderInsert(sb, p, n) DynArrayInsert(sb, p, n)
#define StringBuilderUnorderedInsert(sb, p, n) DynArrayUnorderedInsert(sb, p, n)
#define StringBuilderPop(sb, p) DynArrayPop(sb, p)
#define StringBuilderUnorderedPop(sb, p) DynArrayUnorderedPop(sb, p)

typedef enum StringMatchFlag StringMatchFlag;
enum StringMatchFlag
{
  CASE_SENSITIVE   = 1 << 0,
  CASE_INSENSITIVE = 1 << 1
};

typedef struct String String;
struct String
{
  u8 *value;
  u64 count;
};

typedef struct StringBuilder StringBuilder;
struct StringBuilder
{
  _DynArrayHeader_;
  union
  {
    u8 *items;
    u8 *value;
  };
};

internal b32 CharIsSpace(u8 c);
internal b32 CharIsUpper(u8 c);
internal b32 CharIsLower(u8 c);
internal b32 CharIsAlpha(u8 c);
internal b32 CharIsSlash(u8 c);
internal b32 CharIsDigit(u8 c, u32 base);
internal u8  CharMakeLower(u8 c);
internal u8  CharMakeUpper(u8 c);
internal u8  CharCorrectSlash(u8 c);

internal b32    StringMatch(String a, String b, StringMatchFlag flag);
internal String StringFromCstr(u8 *s);
internal String StringSub(String s, i64 begin, i64 size);
internal String StringFmt(u8 *fmt, ...);



