internal f32
ZINK_LinearInterpolate(f32 point_a, f32 point_b, f32 factor)
{
  return point_a + factor * (point_b - point_a);
}

internal f32
ZINK_Clamp(f32 value, f32 min, f32 max)
{
  f32 result = value < min ? min : value;
  return result > max ? max : result;
}

internal f32
ZINK_GetDeltaTime()
{
  f32 current_tick = SDL_GetTicks();
  local f32 last_tick = 0.0f;

  if (current_tick == 0) last_tick = current_tick;
  
  f32 delta_time = (current_tick - last_tick) / 1000.0f;
  last_tick = current_tick;
  
  return delta_time;
}

internal void
ZINK_DrawCircle(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius)
{
  int x = radius;
  int y = 0;
  int midpoint = 0;

  while (x >= y)
  {
    SDL_RenderPoint(renderer, center_x + x, center_y + y);
    SDL_RenderPoint(renderer, center_x + y, center_y + x);
    SDL_RenderPoint(renderer, center_x - y, center_y + x);
    SDL_RenderPoint(renderer, center_x - x, center_y + y);
    SDL_RenderPoint(renderer, center_x - x, center_y - y);
    SDL_RenderPoint(renderer, center_x - y, center_y - x);
    SDL_RenderPoint(renderer, center_x + y, center_y - x);
    SDL_RenderPoint(renderer, center_x + x, center_y - y);

    if (midpoint <= 0)
    {
      y++;
      midpoint += 2*y + 1;
    }
    else
    {
      x--;
      midpoint -= 2*x + 1;
    }
  }
}

internal void
ZINK_DrawCircleFilledDeprecated(SDL_Renderer *renderer, i32 center_x, i32 center_y, i32 radius)
{
  i32 x = 0;
  i32 y = radius;
  i32 midpoint = 5 - 4 * radius;

  while (x <= y)
  {
    if (midpoint > 0)
    {
      SDL_RenderLine(renderer, center_x - x, center_y - y, center_x + x, center_y - y);
      SDL_RenderLine(renderer, center_x - x, center_y + y, center_x + x, center_y + y);                       
      y--;
      midpoint -= 8 * y;
    }
    SDL_RenderLine(renderer, center_x - y, center_y - x, center_x + y, center_y - x);
    SDL_RenderLine(renderer, center_x - y, center_y + x, center_x + y, center_y + x);
                
    x++;
    midpoint += 8 * x + 4;
  }
}

internal void
ZINK_DrawCircleFilledCPU(u32 *pixels, i32 pitch, i32 width, i32 height, i32 center_x, i32 center_y, i32 radius)
{
  for (i32 col = -radius; col <= radius; ++col)
  {
    for (i32 row = -radius; row <= radius; ++row)
    {
      if (row * row + col * col <= radius * radius)
      {
        i32 pixel_x = center_x + row;
        i32 pixel_y = center_y + col;

        if (pixel_x >= 0 && pixel_y >= 0 &&
            pixel_x < width && pixel_y < height)
        {
          i32 idx = pixel_y * pitch + pixel_x;
          pixels[idx] = 0xDC143CFF;
        }
      }
    }
  }
}

internal void
ZINK_EraseCircleFilledCPU(u32 *pixels, u32 *original, i32 pitch, i32 width, i32 height, i32 center_x, i32 center_y, i32 radius)
{
  for (i32 col = -radius; col <= radius; ++col)
  {
    for (i32 row = -radius; row <= radius; ++row)
    {
      if (row * row + col * col <= radius * radius)
      {
        i32 pixel_x = center_x + row;
        i32 pixel_y = center_y + col;

        if (pixel_x >= 0 && pixel_y >= 0 &&
            pixel_x < width && pixel_y < height)
        {
          i32 idx = pixel_y * pitch + pixel_x;
          pixels[idx] = original[idx];
        }
      }
    }
  }
}
