global b32 g_running;

// TODO: remove these global variables
global f32 drag_start_mouse_x;
global f32 drag_start_mouse_y;
global f32 drag_start_target_x;
global f32 drag_start_target_y;

global i32 brush_size = 4.0f;
global f32 sprite_scale_factor = 0.5;

void
ZINK_TriggerMainLoop(i32 width, i32 height, u8 *title, u8 *image_path)
{
  ZINK_Context zink_context = {0};
  ZINK_InputState input = {0};
  
  ZINK_InitContext(&zink_context, width, height, title, "direct3d11", true, image_path);

  u64 last_tick = 0;
  g_running = true;
  while (g_running)
  {
    ZINK_ResetInputState(&input);
    u64 current_tick = SDL_GetTicks();
    f32 delta_time = ZINK_GetDeltaTime();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ZINK_DispatchEvent(&event, &input);
    }
    
    ZINK_UpdateAndRender(&zink_context, &input, delta_time);

    zink_context.fps++;
    if (current_tick > last_tick + 1000)
    {
      last_tick = current_tick;
      zink_context.fps = 0;
    }
  }
  ZINK_DestroyContext(&zink_context);
  SDL_Quit();
}

internal b32
ZINK_InitContext(ZINK_Context *ctx, i32 width, i32 height, u8 *title, u8 *driver, b32 vsync_flag, u8 *image_path)
{
  // NOTE: Window Setup
  ctx->window_width = width;
  ctx->window_height = height;
  ctx->window_title = title;
  ctx->driver = driver;
  ctx->vsync = vsync_flag;
  ctx->initialized = true;
  ctx->fps = 0;
  
  SDL_Require(SDL_Init(SDL_INIT_VIDEO));
  
  i32 window_flags = SDL_WINDOW_INPUT_FOCUS |
                     SDL_WINDOW_MOUSE_FOCUS |
                     SDL_WINDOW_BORDERLESS;
  ctx->window = SDL_CreateWindow(ctx->window_title,
                                     ctx->window_width,
                                     ctx->window_height,
                                     window_flags);
  SDL_Require(ctx->window);

  SDL_ShowCursor();
  if (!SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT))) { return false; }
  if (!SDL_SetWindowFocusable(ctx->window, true))                    { return false; }
  if (!SDL_RaiseWindow(ctx->window))                                 { return false; }

  ctx->renderer = SDL_CreateRenderer(ctx->window, ctx->driver);
  SDL_Require(ctx->renderer);
  SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);     
  
  if (vsync_flag) { SDL_SetRenderVSync(ctx->renderer, ZINC_VSYNC_ENABLE); }
  else            { SDL_SetRenderVSync(ctx->renderer, ZINC_VSYNC_DISABLE); }
        
  // NOTE: Screenshot texture
  SDL_Surface *tmp;
  DeferScope(tmp = IMG_Load(image_path), SDL_DestroySurface(tmp))
  {
    SDL_Require(tmp);
    ctx->surface = SDL_ConvertSurface(tmp, SDL_PIXELFORMAT_RGBA8888);
    SDL_Require(ctx->surface);
  }

  i32 pixel_count = ctx->surface->w * ctx->surface->h;
  ctx->original_pixels = (u32 *)malloc(pixel_count * sizeof(u32));
  memcpy(ctx->original_pixels, ctx->surface->pixels, pixel_count * sizeof(u32));
        
  ctx->texture_width  = ctx->surface->w;
  ctx->texture_height = ctx->surface->h;  
  ctx->texture = SDL_CreateTexture(ctx->renderer,
                                       SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       ctx->texture_width,
                                       ctx->texture_height);
  SDL_Require(ctx->texture);
  SDL_SetTextureBlendMode(ctx->texture, SDL_BLENDMODE_BLEND);

  // NOTE: Copy initial image into screenshot texture
  SDL_Texture *temp;
  DeferScope(temp = SDL_CreateTextureFromSurface(ctx->renderer, ctx->surface), SDL_DestroyTexture(temp))
  {
    SDL_SetRenderTarget(ctx->renderer, ctx->texture);
    SDL_RenderTexture(ctx->renderer, temp, NULL, NULL);         
  }

  ctx->dest.w = ctx->window_width;
  ctx->dest.h = ctx->window_height;

  // NOTE: Sprite texture
  u8 *sprite_path = "..\\assets\\keys.png";
  DeferScope(tmp = IMG_Load(sprite_path), SDL_DestroySurface(tmp))
  {
    SDL_Require(tmp);
    ctx->sprite = SDL_CreateTextureFromSurface(ctx->renderer, tmp);
    SDL_Require(ctx->texture);
  }

  ctx->camera.offset.x = ctx->window_width * 0.5f;
  ctx->camera.offset.y = ctx->window_height * 0.5f;
  ctx->camera.target.x = ctx->texture_width * 0.5f;
  ctx->camera.target.y = ctx->texture_height * 0.5f;
  ctx->camera.rotation = 0.0f;
  ctx->camera.zoom = 1.0f;
  ctx->camera.zoom_target = 1.0f;

  return true;
}

internal void
ZINK_ProcessInput(ZINK_Context *ctx, ZINK_InputState *input, f32 delta_time)
{
  if (input->kbd_down[SDL_SCANCODE_ESCAPE]) { g_running = false; }
  if (input->kbd_pressed[SDL_SCANCODE_E])   { input->brush_mode ^= 1; }
  if (input->kbd_down[SDL_SCANCODE_F])      { brush_size = ZINK_Clamp(++brush_size, 4.0f, 30.0f); }
  if (input->kbd_down[SDL_SCANCODE_D])      { brush_size = ZINK_Clamp(--brush_size, 4.0f, 30.0f); }
  if (input->mouse_down[SDL_BUTTON_LEFT])
  {
    input->last_world_x = input->world_x;
    input->last_world_y = input->world_y;
  }

  ZINK_UpdateCamera(&ctx->camera,
                    input,
                    ctx->texture_width,
                    ctx->texture_height,
                    delta_time);    

  ctx->dest.x = ctx->camera.offset.x - ctx->camera.target.x * ctx->camera.zoom;
  ctx->dest.y = ctx->camera.offset.y - ctx->camera.target.y * ctx->camera.zoom;    
  ctx->dest.w = ctx->texture_width * ctx->camera.zoom;
  ctx->dest.h = ctx->texture_height * ctx->camera.zoom;     
}

internal void
ZINK_StrokeOnCanvas(ZINK_Context *ctx, ZINK_InputState *input)
{
  if (!input->mouse_down[SDL_BUTTON_LEFT]) { return; }
  
  u32 *pixels          = (u32 *)ctx->surface->pixels;
  u32 *original_pixels = ctx->original_pixels;
  i32  pitch           = ctx->surface->pitch / sizeof(u32);      

  f32 dt_x     = input->world_x - input->last_world_x;
  f32 dt_y     = input->world_y - input->last_world_y;
  f32 distance = sqrtf((dt_x * dt_x) + (dt_y * dt_y));

  SDL_LockSurface(ctx->surface);
  {
    if (distance <= 0.0f) { return; }
    
    f32 inverse     = 1.0f / distance;
    f32 direction_x = dt_x * inverse;
    f32 direction_y = dt_y * inverse;
    
    for (f32 idx = 0.0f; idx <= distance; idx += 0.1f)
    {
      f32 point_x = input->last_world_x + direction_x * idx;
      f32 point_y = input->last_world_y + direction_y * idx;
      if (input->brush_mode == ERASE)
      {
        ZINK_EraseCircleFilledCPU(pixels, original_pixels,
                                  pitch,
                                  ctx->texture_width,
                                  ctx->texture_height,
                                  (i32)point_x,
                                  (i32)point_y,
                                  brush_size);
      }
      else
      {
        ZINK_DrawCircleFilledCPU(pixels,
                                 pitch,
                                 ctx->texture_width,
                                 ctx->texture_height,
                                 (i32)point_x,
                                 (i32)point_y,
                                 brush_size);
      }
    }
    SDL_UnlockSurface(ctx->surface);
    
    input->last_world_x = input->world_x;
    input->last_world_y = input->world_y;           
  }
}

internal void
ZINK_RenderToolbar(ZINK_Context *ctx, ZINK_InputState *input)
{
  SDL_FRect toolbar_background = {ctx->window_width - 32, ctx->window_height - (32 * 6), 66.0f, 32.0f * 6};
  SDL_SetRenderDrawColor(ctx->renderer, 29, 29, 29, 150);             
  SDL_RenderFillRect(ctx->renderer, &toolbar_background);

  // NOTE(arka): Key driven buttons: ESC, R, D, F
  ZINK_Toolbar buttons[] = {
    {0, SDL_SCANCODE_ESCAPE, 1},
    {1, SDL_SCANCODE_R,      2},
    {3, SDL_SCANCODE_D,      4},
    {4, SDL_SCANCODE_F,      5}
  };
  for (i32 idx = 0; idx < ArrayCount(buttons); idx++)
  {
    ZINK_Toolbar *btn = &buttons[idx];
    SDL_FRect src = {input->kbd_down[btn->key] ? 64.0f : 0.0f, (f32)(64 * btn->sprite_row), 64, 64};
    SDL_FRect dst = {
      ctx->window_width - 32, 
      ctx->window_height - 32 * btn->slot,
      sprite_scale_factor * 64,
      sprite_scale_factor * 64
    };
    SDL_RenderTexture(ctx->renderer, ctx->sprite, &src, &dst);
  }
  
  // NOTE(arka): State driven buttons: E
  SDL_FRect src_e = {input->brush_mode == ERASE ? 64.0f : 0.0f, 64 * 2, 64, 64};
  SDL_FRect dst_e = {
    ctx->window_width - 32,
    ctx->window_height - 32 * 3,
    sprite_scale_factor * 64,
    sprite_scale_factor * 64
  };
  SDL_RenderTexture(ctx->renderer, ctx->sprite, &src_e, &dst_e);

  // NOTE(arka): Mouse buttons
  SDL_FRect src_mouse_base  = {0.0f,  64.0f * 5, 64.0f, 64.0f};
  SDL_FRect dst_mouse = {
    ctx->window_width - 32,
    ctx->window_height - 32 * 6,
    64 * sprite_scale_factor,
    64 * sprite_scale_factor
  };
  SDL_RenderTexture(ctx->renderer, ctx->sprite, &src_mouse_base,  &dst_mouse);
  
  SDL_FRect src_mouse_left  = {64.0f, 64.0f * 5, 64.0f, 64.0f};
  SDL_FRect src_mouse_right = {64.0f, 64.0f * 6, 64.0f, 64.0f};
  SDL_FRect src_mouse_wheel = {64.0f, 64.0f * 7, 64.0f, 64.0f};
  
  if (input->mouse_down[SDL_BUTTON_LEFT])  { SDL_RenderTexture(ctx->renderer, ctx->sprite, &src_mouse_left,  &dst_mouse); }
  if (input->mouse_down[SDL_BUTTON_RIGHT]) { SDL_RenderTexture(ctx->renderer, ctx->sprite, &src_mouse_right, &dst_mouse); }
  if (input->wheel != 0)                   { SDL_RenderTexture(ctx->renderer, ctx->sprite, &src_mouse_wheel, &dst_mouse); }
}

internal void
ZINK_UpdateAndRender(ZINK_Context *ctx, ZINK_InputState *input, f32 delta_time)
{
  ZINK_ProcessInput(ctx, input, delta_time);
  
  // NOTE:(Rendering Layer): Painting
  SDL_SetRenderTarget(ctx->renderer, ctx->texture);
  SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
  ZINK_StrokeOnCanvas(ctx, input);
        
  // NOTE(Rendering Layer): Clear background
  SDL_SetRenderDrawColor(ctx->renderer, 29, 29, 29, 255);
  SDL_RenderClear(ctx->renderer);

  // NOTE(Canvas Layer): Draw texture
  SDL_UpdateTexture(ctx->texture, NULL, ctx->surface->pixels, ctx->surface->pitch);
  SDL_RenderTexture(ctx->renderer, ctx->texture, NULL, &ctx->dest);

  // NOTE(Rendering Layer): Brush preview
  SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 150);  
  ZINK_DrawCircle(ctx->renderer, input->mouse_x, input->mouse_y, brush_size);
  
  // NOTE(Toolbar Layer)
  ZINK_RenderToolbar(ctx, input);
  
  // NOTE(Rendering Layer): Button, Font etc
#if ZINK_DEBUG_MODE
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 5,  "Brush Size: %d", brush_size);
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 15, "Mouse Drag: %d", input->drag);
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 25, "Mouse LClick: %d", input->mouse_down[SDL_BUTTON_LEFT]);
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 35, "Mouse RClick: %d", input->mouse_down[SDL_BUTTON_RIGHT]);
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 45, "Current Mouse (X, Y): (%f, %f)", input->world_x, input->world_y);
  SDL_RenderDebugTextFormat(ctx->renderer, 5, 55, "Last Mouse (X, Y)   : (%f, %f)", input->last_world_x, input->last_world_y);
#endif

  SDL_RenderPresent(ctx->renderer);
}

internal void
ZINK_DestroyContext(ZINK_Context *ctx)
{
  if (!ctx->initialized) return;

  free(ctx->original_pixels);
  ctx->original_pixels = 0;
        
  SDL_DestroyTexture(ctx->texture);
  SDL_DestroySurface(ctx->surface);   
  SDL_DestroyRenderer(ctx->renderer);
  SDL_DestroyWindow(ctx->window);
}

// NOTE(arka): depricated fn
// internal b32
// ZINK_InitInputState(ZINK_InputState *input)
// {
//   input->brush_mode = DRAW;
//   memset(input->mouse_down,     0, sizeof(input->mouse_down[0]) * 5);
//   memset(input->mouse_released, 0, sizeof(input->mouse_released[0]) * 5);
//   memset(input->kbd_down,       0, sizeof(input->kbd_down[0]) * SDL_SCANCODE_COUNT);
//   memset(input->kbd_released,   0, sizeof(input->kbd_released[0]) * SDL_SCANCODE_COUNT);
// 
//   return true;
// }

internal void
ZINK_ResetInputState(ZINK_InputState *input)
{
  MemoryZero(input->kbd_pressed, sizeof(input->kbd_pressed));
  MemoryZero(input->kbd_released, sizeof(input->kbd_released));
  MemoryZero(input->mouse_pressed, sizeof(input->mouse_pressed));
  MemoryZero(input->mouse_released, sizeof(input->mouse_released));

  input->wheel = 0;
}

internal void
ZINK_DispatchEvent(SDL_Event *event, ZINK_InputState *input)
{
  SDL_TextInputEvent   text     = event->text;
  SDL_KeyboardEvent    keyboard = event->key;
  SDL_MouseButtonEvent mouse    = event->button;
  SDL_MouseWheelEvent  wheel    = event->wheel;
  SDL_MouseMotionEvent motion   = event->motion;
  
  switch (event->type)
  {
    case SDL_EVENT_QUIT:
    {
      g_running = false;
    } break;
                
    ////////////////////////////////
    // NOTE: KEYBOARD EVENTS
    //
    case SDL_EVENT_KEY_DOWN:
    {
      u32 sc = keyboard.scancode;
      KeyboardDown(sc, input);
    } break;

    case SDL_EVENT_KEY_UP:
    {
      u32 sc = keyboard.scancode;
      KeyboardUp(sc, input);
    } break;

    ////////////////////////////////
    // NOTE: TEXT EVENTS
    //
    case SDL_EVENT_TEXT_INPUT:
    {
      input->ascii_char = *text.text;
    } break;
                
    ////////////////////////////////
    // NOTE: MOUSE EVENTS
    //
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
      u32 btn = mouse.button;
      MouseDown(btn, input);
      input->drag = input->mouse_down[SDL_BUTTON_RIGHT];
    } break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
      u32 btn = mouse.button;
      MouseUp(btn, input);
      input->drag = input->mouse_down[SDL_BUTTON_RIGHT];
    } break;

    case SDL_EVENT_MOUSE_WHEEL:
    {
      input->wheel += wheel.y;                  
    } break;

    case SDL_EVENT_MOUSE_MOTION:
    {
      input->mouse_x = motion.x;
      input->mouse_y = motion.y;
    } break;

    default:
    {           
    } break;
  }
}

internal void
ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, f32 texture_width, f32 texture_height, f32 delta_time)
{
  input->world_x = (input->mouse_x - cam->offset.x) / cam->zoom + cam->target.x;
  input->world_y = (input->mouse_y - cam->offset.y) / cam->zoom + cam->target.y;
  
  if (input->mouse_pressed[SDL_BUTTON_RIGHT])
  {
    drag_start_mouse_x = input->mouse_x;
    drag_start_mouse_y = input->mouse_y;
    drag_start_target_x = cam->target.x;
    drag_start_target_y = cam->target.y;
  }

  if (input->wheel)
  {
    cam->offset.x = input->mouse_x;
    cam->offset.y = input->mouse_y;
    cam->target.x = input->world_x;
    cam->target.y = input->world_y;
    
    local f32 zoom_factor = 1.1f;
    if (input->wheel > 0) cam->zoom_target *= zoom_factor;
    if (input->wheel < 0) cam->zoom_target /= zoom_factor;
    
    cam->zoom_target = ZINK_Clamp(cam->zoom_target, 0.5f, 7.0f);    
  }
  
  if (input->drag)
  {
    f32 dx = (input->mouse_x - drag_start_mouse_x) / cam->zoom;
    f32 dy = (input->mouse_y - drag_start_mouse_y) / cam->zoom;
    cam->target.x = drag_start_target_x - dx;
    cam->target.y = drag_start_target_y - dy;    
  }

  if (input->kbd_down[SDL_SCANCODE_R])
  {
    ZINK_ResetCamera(cam, texture_width, texture_height);
  }  

  cam->zoom = ZINK_LinearInterpolate(cam->zoom, cam->zoom_target, 8.0f * delta_time);
}

internal void
ZINK_ResetCamera(ZINK_Camera2D *cam, f32 texture_width, f32 texture_height)
{
  cam->offset.x    = texture_width * 0.5f;
  cam->offset.y    = texture_height * 0.5f;    
  cam->target.x    = texture_width * 0.5f;
  cam->target.y    = texture_height * 0.5f;
  cam->zoom        = 1.0f;
  cam->zoom_target = 1.0f;  
}
