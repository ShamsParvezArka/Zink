#ifndef ARRAY_H
#define ARRAY_H

#define DYN_ARRAY_GROWTH_FACTOR 1.25
#define DYN_ARRAY_INITIAL_SIZE KB(1)

#define _DynArrayHeader_ struct { u64 count; u64 capacity; }

#define DynArrayGrow(da)                                                \
  do {                                                                  \
    (da).capacity = (da).capacity ?                                     \
      (u64)((da).capacity * DYN_ARRAY_GROWTH_FACTOR) :                  \
      DYN_ARRAY_INITIAL_SIZE;                                           \
                                                                        \
    void *new_chunk = realloc((da).items, (da).capacity * sizeof(*(da).items)); \
    assert(new_chunk && "Out of memory. Buy more ram! LOL");            \
    (da).items = new_chunk;                                             \
  } while (0);                                                          \

#define DynArrayPush(da, n)                     \
  do {                                          \
    if ((da).count >= (da).capacity)            \
    {                                           \
      DynArrayGrow(da);                         \
    }                                           \
    (da).items[(da).count++] = n;               \
  } while (0);                                  \
  
#define DynArrayInsert(da, p, n)                                \
  do {                                                          \
    assert(((p) <= (da).count) && "Unable to insert item");     \
    if ((da).count >= (da).capacity)                            \
    {                                                           \
      DynArrayGrow(da);                                         \
    }                                                           \
                                                                \
    for (u64 idx = (da).count; idx > (p); idx--)                \
    {                                                           \
      (da).items[idx] = (da).items[idx - 1];                    \
    }                                                           \
    (da).items[(p)] = (n);                                      \
    (da).count++;                                               \
  } while (0);                                                  \


#define DynArrayUnorderedInsert(da, p, n)                       \
  do {                                                          \
    assert(((p) <= (da).count) && "Unable to insert item");     \
    if ((da).count >= (da).capacity)                            \
    {                                                           \
      DynArrayGrow(da);                                         \
    }                                                           \
                                                                \
    (da).items[(da).count++] = (da).items[(p)];                 \
    (da).items[(p)] = (n);                                      \
  } while (0);                                                  \

#define DynArrayPop(da, p)                              \
  do {                                                  \
    assert(((p) < (da).count) && "Unable to pop item"); \
                                                        \
    for (u64 idx = (p); idx < (da).count - 1; idx++)    \
    {                                                   \
      (da).items[idx] = (da).items[idx + 1];            \
    }                                                   \
    (da).count--;                                       \
  } while (0);                                          \

#define DynArrayUnorderedPop(da, p)                     \
  do {                                                  \
    assert(((p) < (da).count) && "Unable to pop item"); \
    (da).items[(p)] = (da).items[--(da).count];         \
  } while (0);                                          \

#define DynArrayFree(da)                        \
  do {                                          \
    free(da.items);                             \
    da.count = 0;                               \
    da.capacity = 0;                            \
  } while (0);                                  \

#endif // ARRAY_H
