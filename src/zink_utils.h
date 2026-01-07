#ifndef ZINK_UTILS_H
#define ZINK_UTILS_H

_internal F32  ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor);
_internal F32  ZINK_Clamp(F32 value, F32 min, F32 max);
_internal F32  ZINK_GetDeltaTime();
_internal void ZINK_DrawCircle(SDL_Renderer *renderer, I32 center_x, I32 center_y, I32 radius);
_internal void ZINK_DrawCircleFilled(SDL_Renderer *renderer, I32 center_x, I32 center_y, I32 radius);
_internal void ZINK_DrawCircleFilledCPU(U32 *pixels, I32 pitch, I32 width, I32 height, I32 center_x, I32 center_y, I32 radius);

#endif // ZINK_UTILS_H
