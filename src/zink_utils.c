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

_internal void
ZINK_DrawCircle(SDL_Renderer *renderer, I32 center_x, I32 center_y, I32 radius)
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

_internal void
ZINK_DrawCircleFilledDeprecated(SDL_Renderer *renderer, I32 center_x, I32 center_y, I32 radius)
{
	I32 x = 0;
	I32 y = radius;
	I32 midpoint = 5 - 4 * radius;

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

_internal void
ZINK_DrawCircleFilledCPU(U32 *pixels, I32 pitch, I32 width, I32 height, I32 center_x, I32 center_y, I32 radius)
{
	for (I32 col = -radius; col <= radius; ++col)
	{
		for (I32 row = -radius; row <= radius; ++row)
		{
			if (row * row + col * col <= radius * radius)
			{
				I32 pixel_x = center_x + row;
				I32 pixel_y = center_y + col;

				if (pixel_x >= 0 && pixel_y >= 0 &&
						pixel_x < width && pixel_y < height)
				{
					I32 idx = pixel_y * pitch + pixel_x;
					pixels[idx] = 0xFFFFFFFF;
				}
			}
		}
	}
}

_internal void
ZINK_EraseCircleFilledCPU(U32 *pixels, U32 *original, I32 pitch, I32 width, I32 height, I32 center_x, I32 center_y, I32 radius)
{
	for (I32 col = -radius; col <= radius; ++col)
	{
		for (I32 row = -radius; row <= radius; ++row)
		{
			if (row * row + col * col <= radius * radius)
			{
				I32 pixel_x = center_x + row;
				I32 pixel_y = center_y + col;

				if (pixel_x >= 0 && pixel_y >= 0 &&
					  pixel_x < width && pixel_y < height)
				{
					I32 idx = pixel_y * pitch + pixel_x;
					pixels[idx] = original[idx];
				}
			}
		}
	}
}
