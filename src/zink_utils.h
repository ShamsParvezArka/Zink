#ifndef ZINK_UTILS_H
#define ZINK_UTILS_H

internal f32  ZINK_LinearInterpolate(f32 point_a, f32 point_b, f32 factor);
internal f32  ZINK_Clamp(f32 value, f32 min, f32 max);
internal f32  ZINK_GetDeltaTime();
internal void ZINK_DrawCircle(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius);
internal void ZINK_DrawCircleFilledDeprecated(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius);
internal void ZINK_DrawCircleFilledCPU(u32 *pixels, i32 pitch, i32 width, i32 height, i32 center_x, i32 center_y, i32 radius);
internal void ZINK_EraseCircleFilledCPU(u32 *pixels, u32 *original, i32 pitch, i32 width, i32 height, i32 center_x, i32 center_y, i32 radius);

#endif // ZINK_UTILS_H
