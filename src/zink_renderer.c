#include "zink_renderer.h"

_global B32 running;
_global B32 dragging;

_global F32 world_mouse_x;
_global F32 world_mouse_y;

_global F32 drag_start_mouse_x;
_global F32 drag_start_mouse_y;
_global F32 drag_start_target_x;
_global F32 drag_start_target_y;

_internal B32 ZINK_RendererInit(ZINK_Renderer *renderer_handle, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag)
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
  
  I32 window_flags = SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_BORDERLESS;
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

_internal B32 ZINK_ContextInit(ZINK_Renderer *renderer_handle, ZINK_Context *context)
{
	String8 image_path = "..\\assets\\screenshot.bmp";

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

  return true;
}

_internal void ZINK_ContextDestroy(ZINK_Context *context)
{
  SDL_DestroySurface(context->surface);
  SDL_DestroyTexture(context->texture);
}

_internal void ZINK_RendererDestroy(ZINK_Renderer *renderer_handle)
{
  if (renderer_handle->initialized)
  {
    SDL_DestroyRenderer(renderer_handle->renderer);
    SDL_DestroyWindow(renderer_handle->window);    
  }
}

void ZINK_TriggerMainLoop(I32 width, I32 height, String8 title)
{
  ZINK_Renderer renderer_handle = {};
  String8 driver = "direct3d11";
  ZINK_RendererInit(&renderer_handle, width, height, title, driver, true);

  ZINK_Context context = {};
  ZINK_ContextInit(&renderer_handle, &context);
  
  running = true;
  
  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ZINK_ManageInputEvent(&context, event);
    }

    context.camera.zoom = ZINK_LinearInterpolate(context.camera.zoom, context.camera.zoom_target, 0.05f);  

    context.dest.x = context.camera.offset.x - context.camera.target.x * context.camera.zoom;
    context.dest.y = context.camera.offset.y - context.camera.target.y * context.camera.zoom;    
    context.dest.w = context.texture_width * context.camera.zoom;
    context.dest.h = context.texture_height * context.camera.zoom;

    SDL_SetRenderDrawColor(renderer_handle.renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer_handle.renderer);
    SDL_RenderTexture(renderer_handle.renderer, context.texture, NULL, &context.dest);
    SDL_RenderPresent(renderer_handle.renderer);
  }

	ZINK_ContextDestroy(&context);
  ZINK_RendererDestroy(&renderer_handle);
  SDL_Quit();
}

_internal void ZINK_ManageInputEvent(ZINK_Context *context, SDL_Event event)
{
  switch (event.type)
  {
    case SDL_EVENT_QUIT:
    {
      running = false;
    } break;
    
    case SDL_EVENT_KEY_DOWN:
    {
      SDL_Keycode input = event.key.key;
      if (input == SDLK_ESCAPE) running = false;
    } break;

    case SDL_EVENT_MOUSE_WHEEL:
    {
      F32 mouse_x;
      F32 mouse_y;
      SDL_GetMouseState(&mouse_x, &mouse_y);
      
      world_mouse_x = (mouse_x - context->camera.offset.x) / context->camera.zoom + context->camera.target.x;
      world_mouse_y = (mouse_y - context->camera.offset.y) / context->camera.zoom + context->camera.target.y;

      context->camera.offset.x = mouse_x;
      context->camera.offset.y = mouse_y;
      context->camera.target.x = world_mouse_x;
      context->camera.target.y = world_mouse_y;

      if (event.wheel.y > 0) context->camera.zoom_target += 0.15f;
      if (event.wheel.y < 0) context->camera.zoom_target -= 0.15f;
      context->camera.zoom_target = ZINK_Clamp(context->camera.zoom_target, 0.5f, 7.0f);
    } break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
      if (event.button.button == SDL_BUTTON_LEFT)
      {
        dragging = true;
        SDL_GetMouseState(&drag_start_mouse_x, &drag_start_mouse_y);
        drag_start_target_x = context->camera.target.x;
        drag_start_target_y = context->camera.target.y;
      }
    } break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
      if (event.button.button == SDL_BUTTON_LEFT)
      {
        dragging = false;
      }
    } break;

    case SDL_EVENT_MOUSE_MOTION:
    {
      if (dragging)
      {
        F32 mouse_x;
        F32 mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        F32 dx = (mouse_x - drag_start_mouse_x) / context->camera.zoom;
        F32 dy = (mouse_y - drag_start_mouse_y) / context->camera.zoom;

        context->camera.target.x = drag_start_target_x - dx;
        context->camera.target.y = drag_start_target_y - dy;
      }
    } break;


    default:
    {
    } break;
  }
}

_internal F32 ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor)
{
  return point_a + factor * (point_b - point_a);
}

_internal F32  ZINK_Clamp(F32 value, F32 min, F32 max)
{
  F32 result = value < min ? min : value;
  return result > max ? max : result;
}
