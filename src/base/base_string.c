internal b32
CharIsSpace(u8 c)
{
  return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

internal b32
CharIsUpper(u8 c)
{
  return ('A' <= c && c <= 'Z');
}

internal b32
CharIsLower(u8 c)
{
  return ('a' <= c && c <= 'z');
}

internal b32
CharIsAlpha(u8 c)
{
  return (CharIsUpper(c) || CharIsLower(c));
}

internal b32
CharIsSlash(u8 c)
{
  return (c == '/' || c == '\\');
}

internal b32
CharIsDigit(u8 c, u32 base)
{
  b32 result = 0;
  if(0 < base && base <= 16)
  {
    u8 val = integer_symbol_reverse[c];
    if(val < base)
    {
      result = 1;
    }
  }
  return result;
}

internal u8
CharMakeLower(u8 c)
{
  if(CharIsUpper(c))
  {
    c += ('a' - 'A');
  }
  return c;
}

internal u8
CharMakeUpper(u8 c)
{
  if(CharIsLower(c))
  {
    c += ('A' - 'a');
  }
  return c;
}

internal u8
CharCorrectSlash(u8 c)
{
  if(CharIsSlash(c))
  {
    c = '/';
  }
  return c;
}

internal String
StringFromCstr(u8 *s)
{
  String result = {};
  result.value = s;
  result.count = MemoryCstrLength(s);
        
  return result;
}

internal String
StringSub(String s, i64 begin, i64 size)
{
  begin = Min(Max(begin, 0), s.count);
  size = Min(Max(size, 0), s.count - begin);     
        
  String result = {};   
  result.value = s.value + begin;
  result.count = size;
        
  return result;
}

internal b32
StringMatch(String a, String b, StringMatchFlag flag)
{
  b32 result = 0;

  if (a.count == b.count && flag == CASE_SENSITIVE)
  {
    result = MemoryMatch(a.value, b.value, b.count);
  }
  if (a.count == b.count && flag == CASE_INSENSITIVE)
  {
    result = 1;
    for (u64 idx = 0; idx < a.count; idx++)
    {
      u8 at = CharMakeLower(a.value[idx]);
      u8 bt = CharMakeLower(b.value[idx]);
      if (at != bt)
      {
        result = 0;
        break;
      }
    }
  }

  return result;        
}

internal String
StringFmt(u8 *fmt, ...)
{
  va_list args;
  va_list args_backup;
  va_start(args, fmt);
  va_copy(args_backup, args);

  u32 needed_bytes = vsnprintf(0, 0, fmt, args) + 1;

  String result = {};
  result.value = (u8 *)malloc(needed_bytes);
  result.count = vsnprintf((u8 *)result.value, needed_bytes, fmt, args_backup);

  va_end(args);
  va_end(args_backup);  
  return result;
}
