#include "zink_core.h"

_internal F32
ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor)
{
  return point_a + factor * (point_b - point_a);
}

_internal F32
ZINK_Clamp(F32 value, F32 min, F32 max)
{
  F32 result = value < min ? min : value;
  return result > max ? max : result;
}

_internal F32
ZINK_GetDeltaTime()
{
  F32 current_tick = SDL_GetTicks();
  _local F32 last_tick = 0.0f;

  if (current_tick == 0) last_tick = current_tick;
  
  F32 delta_time = (current_tick - last_tick) / 1000.0f;
  last_tick = current_tick;
  
  return delta_time;
}
