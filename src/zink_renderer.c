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
  ZINK_Renderer renderer_handle = {};
  String8 driver = "direct3d11";
  ZINK_InitRenderer(&renderer_handle, width, height, title, driver, true);

  // NOTE: This is temporary. Here MAX_PATH = 260
  temp_path = malloc(260);
  temp_path = path;

  ZINK_Context context = {};
  ZINK_InitContext(&renderer_handle, &context);

  ZINK_InputState input = {};
  
  running = true;
  while (running)
  {
    F32 delta_time = ZINK_GetDeltaTime();
    
    ZINK_UpdateInputState(&input);
    ZINK_UpdateCamera(&context.camera, &input, delta_time);
    ZINK_UpdateAndRender(&renderer_handle, &context, delta_time);
  }

  ZINK_DestroyContext(&context);
  ZINK_DestroyRenderer(&renderer_handle);
  SDL_Quit();
}

_internal void
ZINK_UpdateAndRender(ZINK_Renderer *renderer_handle, ZINK_Context *context, F32 delta_time)
{
  context->dest.x = context->camera.offset.x - context->camera.target.x * context->camera.zoom;
  context->dest.y = context->camera.offset.y - context->camera.target.y * context->camera.zoom;    
  context->dest.w = context->texture_width * context->camera.zoom;
  context->dest.h = context->texture_height * context->camera.zoom;
  
  SDL_SetRenderDrawColor(renderer_handle->renderer, 29, 29, 29, 255);
  SDL_RenderClear(renderer_handle->renderer);
  SDL_RenderTexture(renderer_handle->renderer, context->texture, NULL, &context->dest);
  SDL_RenderPresent(renderer_handle->renderer);
}

_internal B32
ZINK_InitRenderer(ZINK_Renderer *renderer_handle, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag)
{
  renderer_handle->window_width = width;
  renderer_handle->window_height = height;
  renderer_handle->window_title = title;
  renderer_handle->driver = driver;
  renderer_handle->vsync = vsync_flag;
  renderer_handle->initialized = true;
  
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  I32 window_flags = SDL_WINDOW_INPUT_FOCUS |
                     SDL_WINDOW_MOUSE_FOCUS |
                     SDL_WINDOW_BORDERLESS;
  renderer_handle->window = SDL_CreateWindow(renderer_handle->window_title,
                                             renderer_handle->window_width,
                                             renderer_handle->window_height,
                                             window_flags);
  if (!renderer_handle->window)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  renderer_handle->renderer = SDL_CreateRenderer(renderer_handle->window, renderer_handle->driver);
  if (!renderer_handle->renderer)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  if (vsync_flag) SDL_SetRenderVSync(renderer_handle->renderer, ZINC_VSYNC_ENABLE);
  else            SDL_SetRenderVSync(renderer_handle->renderer, ZINC_VSYNC_DISABLE);
  
  return true;
}

_internal B32
ZINK_InitContext(ZINK_Renderer *renderer_handle, ZINK_Context *context)
{
  String8 image_path = temp_path;

  context->surface = IMG_Load(image_path);
  if (!context->surface)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  context->texture = SDL_CreateTextureFromSurface(renderer_handle->renderer,
                                                  context->surface);
  if (!context->texture)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());    
    return false;
  }

  SDL_GetTextureSize(context->texture,
                     &context->texture_width,
                     &context->texture_height);
  
  context->dest.w = renderer_handle->window_width;
  context->dest.h = renderer_handle->window_height;

  context->camera.offset.x = renderer_handle->window_width * 0.5f;
  context->camera.offset.y = renderer_handle->window_height * 0.5f;
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
ZINK_DestroyRenderer(ZINK_Renderer *renderer_handle)
{
  if (renderer_handle->initialized)
  {
    SDL_DestroyRenderer(renderer_handle->renderer);
    SDL_DestroyWindow(renderer_handle->window);    
  }
}
