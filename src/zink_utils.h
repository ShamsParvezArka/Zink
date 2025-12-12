#ifndef ZINK_UTILS_H
#define ZINK_UTILS_H

_internal F32  ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor);
_internal F32  ZINK_Clamp(F32 value, F32 min, F32 max);
_internal F32  ZINK_GetDeltaTime();

#endif // ZINK_UTILS_H
