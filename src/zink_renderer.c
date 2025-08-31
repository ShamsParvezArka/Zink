#include "zink_core.h"

_global B32 running;

// TODO: remove these global variables
_global F32 world_mouse_x;
_global F32 world_mouse_y;

_global F32 drag_start_mouse_x;
_global F32 drag_start_mouse_y;
_global F32 drag_start_target_x;
_global F32 drag_start_target_y;

_global String8 temp_path;

// TODO: move this filepath to somewhere else. 
void
ZINK_TriggerMainLoop(I32 width, I32 height, String8 title, String8 path)
{
  ZINK_Renderer renderer = {};
  String8 driver = "direct3d11";
  ZINK_InitRenderer(&renderer, width, height, title, driver, true);

  // NOTE: This is temporary. Here MAX_PATH = 260
  temp_path = path;

  ZINK_Context context = {};
  ZINK_InitContext(&renderer, &context);

  ZINK_InputState input = {};
  
  running = true;
  while (running)
  {
    F32 delta_time = ZINK_GetDeltaTime();
    
    ZINK_UpdateInputState(&input);
    ZINK_UpdateCamera(&context.camera,
                      &input,
                      context.texture_width, context.texture_width,
                      delta_time);
    ZINK_UpdateAndRender(&renderer, &context, delta_time);
  }

  ZINK_DestroyContext(&context);
  ZINK_DestroyRenderer(&renderer);
  SDL_Quit();
}

_internal void
ZINK_UpdateAndRender(ZINK_Renderer *renderer, ZINK_Context *context, F32 delta_time)
{
  context->dest.x = context->camera.offset.x - context->camera.target.x * context->camera.zoom;
  context->dest.y = context->camera.offset.y - context->camera.target.y * context->camera.zoom;    
  context->dest.w = context->texture_width * context->camera.zoom;
  context->dest.h = context->texture_height * context->camera.zoom;
  
  SDL_SetRenderDrawColor(renderer->renderer, 29, 29, 29, 255);
  SDL_RenderClear(renderer->renderer);
  SDL_RenderTexture(renderer->renderer, context->texture, NULL, &context->dest);
  SDL_RenderPresent(renderer->renderer);
}

_internal B32
ZINK_InitRenderer(ZINK_Renderer *renderer, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag)
{
  renderer->window_width = width;
  renderer->window_height = height;
  renderer->window_title = title;
  renderer->driver = driver;
  renderer->vsync = vsync_flag;
  renderer->initialized = true;
  
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  I32 window_flags = SDL_WINDOW_INPUT_FOCUS |
                     SDL_WINDOW_MOUSE_FOCUS |
                     SDL_WINDOW_BORDERLESS;
  renderer->window = SDL_CreateWindow(renderer->window_title,
                                             renderer->window_width,
                                             renderer->window_height,
                                             window_flags);
  if (!renderer->window)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_ShowCursor();
  if (!SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT))) return false;
  if (!SDL_SetWindowFocusable(renderer->window, true)) return false;  
  if (!SDL_RaiseWindow(renderer->window)) return false;

  renderer->renderer = SDL_CreateRenderer(renderer->window, renderer->driver);
  if (!renderer->renderer)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  if (vsync_flag) SDL_SetRenderVSync(renderer->renderer, ZINC_VSYNC_ENABLE);
  else            SDL_SetRenderVSync(renderer->renderer, ZINC_VSYNC_DISABLE);
  
  return true;
}

_internal B32
ZINK_InitContext(ZINK_Renderer *renderer, ZINK_Context *context)
{
  String8 image_path = temp_path;

  context->surface = IMG_Load(image_path);
  if (!context->surface)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  context->texture = SDL_CreateTextureFromSurface(renderer->renderer,
                                                  context->surface);
  if (!context->texture)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());    
    return false;
  }

  SDL_GetTextureSize(context->texture,
                     &context->texture_width,
                     &context->texture_height);
  
  context->dest.w = renderer->window_width;
  context->dest.h = renderer->window_height;

  context->camera.offset.x = renderer->window_width * 0.5f;
  context->camera.offset.y = renderer->window_height * 0.5f;
  context->camera.target.x = context->texture_width * 0.5f;
  context->camera.target.y = context->texture_height * 0.5f;
  context->camera.rotation = 0.0f;
  context->camera.zoom = 1.0f;
  context->camera.zoom_target = 1.0f;

  running = true;

  return true;
}

_internal void
ZINK_DestroyContext(ZINK_Context *context)
{
  SDL_DestroySurface(context->surface);
  SDL_DestroyTexture(context->texture);
}

_internal void
ZINK_DestroyRenderer(ZINK_Renderer *renderer)
{
  if (renderer->initialized)
  {
    SDL_DestroyRenderer(renderer->renderer);
    SDL_DestroyWindow(renderer->window);    
  }
}
