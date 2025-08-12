#include <stdlib.h>
#include <string.h>
#include "../include/SDL3/sdl.h"
#include "../include/SDL_image.h"
#include "types.h"
#include "zink_renderer.h"

_global B32 running;
_global B32 dragging;

_global F32 world_mouse_x;
_global F32 world_mouse_y;

_global F32 drag_start_mouse_x;
_global F32 drag_start_mouse_y;
_global F32 drag_start_target_x;
_global F32 drag_start_target_y;

_global F32 last_tick;
_global F32 current_tick;
_global F32 delta;

void ZINK_TriggerMainLoop(I32 width, I32 height, String8 title)
{
  ZINK_Renderer renderer_handle = {};
  String8 driver = "direct3d11";
  ZINK_RendererInit(&renderer_handle, width, height, title, driver, true);

  ZINK_Context context = {};
  ZINK_ContextInit(&renderer_handle, &context);

  ZINK_Toolbar toolbar = {};
  String8 list[3] = {"..\\assets\\move.png",
                     "..\\assets\\draw.png",
                     "..\\assets\\eraser.png"};
  if (!ZINK_ToolbarInit(&renderer_handle, &toolbar, list, 3))
  {
    printf("ZINK_Error: failed to initialize toolbar\n");
  }

  ZINK_InputState input = {};
  
  running = true;
  while (running)
  {
    ZINK_UpdateInputState(&input);

    last_tick = current_tick;
    current_tick = SDL_GetTicks();
    delta = (current_tick - last_tick) / 1000.0f;
    ZINK_UpdateCamera(&context.camera, &input, delta);
    
    context.dest.x = context.camera.offset.x - context.camera.target.x * context.camera.zoom;
    context.dest.y = context.camera.offset.y - context.camera.target.y * context.camera.zoom;    
    context.dest.w = context.texture_width * context.camera.zoom;
    context.dest.h = context.texture_height * context.camera.zoom;

    ZINK_Update(&renderer_handle, &context, delta);
  }

	ZINK_ContextDestroy(&context);
  ZINK_RendererDestroy(&renderer_handle);
  SDL_Quit();
}

_internal void ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, F32 delta)
{
  input->world_x = (input->mouse_x - cam->offset.x) / cam->zoom + cam->target.x;
  input->world_y = (input->mouse_y - cam->offset.y) / cam->zoom + cam->target.y;
  
  if (input->mouse_down[SDL_BUTTON_LEFT])
  {
    drag_start_mouse_x = input->mouse_x;
    drag_start_mouse_y = input->mouse_y;
    drag_start_target_x = cam->target.x;
    drag_start_target_y = cam->target.y;    
  }
  
  if (input->wheel_delta)
  {
    cam->offset.x = input->mouse_x;
    cam->offset.y = input->mouse_y;
    cam->target.x = input->world_x;
    cam->target.y = input->world_y;
    
    _local F32 zoom_factor = 1.1f;
    if (input->wheel_delta > 0) cam->zoom_target *= zoom_factor;
    if (input->wheel_delta < 0) cam->zoom_target /= zoom_factor;
    
    cam->zoom_target = ZINK_Clamp(cam->zoom_target, 0.5f, 7.0f);    
  }
  
  if (input->mouse_drag)
  {
    F32 dx = (input->mouse_x - drag_start_mouse_x) / cam->zoom;
    F32 dy = (input->mouse_y - drag_start_mouse_y) / cam->zoom;
    cam->target.x = drag_start_target_x - dx;
    cam->target.y = drag_start_target_y - dy;    
  }

  cam->zoom = ZINK_LinearInterpolate(cam->zoom, cam->zoom_target, 8.0f * delta);  
}

_internal void ZINK_UpdateInputState(ZINK_InputState *input)
{
	memset(input->mouse_down,     0, sizeof(input->mouse_down[0]) * 5);
	memset(input->mouse_released, 0, sizeof(input->mouse_released[0]) * 5);
  memset(input->key_down,       0, sizeof(input->key_down[0]) * SDL_SCANCODE_COUNT);
  memset(input->key_released,   0, sizeof(input->key_released[0]) * SDL_SCANCODE_COUNT);
  input->wheel_delta = 0.0f;
  
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_EVENT_QUIT:
      {
        running = false;
      } break;
      
      case SDL_EVENT_KEY_DOWN:
      {
        SDL_Keycode key = event.key.key;
        if (key == SDLK_ESCAPE) running = false;
        
        if (key == SDLK_W) input->key_down[SDLK_W] = true;
        if (key == SDLK_D) input->key_down[SDLK_D] = true;
        if (key == SDLK_E) input->key_down[SDLK_E] = true;

        if (key == SDLK_W) input->key_released[SDLK_W] = false;
        if (key == SDLK_D) input->key_released[SDLK_D] = false;
        if (key == SDLK_E) input->key_released[SDLK_E] = false;
      } break;

      case SDL_EVENT_KEY_UP:
      {
        SDL_Keycode key = event.key.key;        
        if (key == SDLK_W) input->key_released[SDLK_W] = true;
        if (key == SDLK_D) input->key_released[SDLK_D] = true;
        if (key == SDLK_E) input->key_released[SDLK_E] = true;

        if (key == SDLK_W) input->key_down[SDLK_W] = true;
        if (key == SDLK_D) input->key_down[SDLK_D] = true;
        if (key == SDLK_E) input->key_down[SDLK_E] = true;
        
      } break;      

      case SDL_EVENT_MOUSE_WHEEL:
      {
				input->wheel_delta = event.wheel.y;
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
          input->mouse_drag = true;
        
				input->mouse_down[event.button.button] = true;
        input->mouse_released[event.button.button] = false;
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
          input->mouse_drag = false;
        input->mouse_released[event.button.button] = true;
        input->mouse_down[event.button.button] = false;        
      } break;

      case SDL_EVENT_MOUSE_MOTION:
      {
        input->mouse_x = event.motion.x;
        input->mouse_y = event.motion.y;
      } break;

      default:
      {
      } break;
    }
  }
}

_internal void ZINK_Update(ZINK_Renderer *renderer_handle,
                           ZINK_Context *context,
                           F32 dt)
{
  SDL_SetRenderDrawColor(renderer_handle->renderer, 29, 29, 29, 255);
  SDL_RenderClear(renderer_handle->renderer);
  SDL_RenderTexture(renderer_handle->renderer, context->texture, NULL, &context->dest);
//  ZINK_ToolbarUpdate(renderer_handle, &toolbar);        
  SDL_RenderPresent(renderer_handle->renderer);
}

_internal void ZINK_DestroyToolbar(ZINK_Toolbar *toolbar)
{
  
}

// TODO: implement ZINK_ToolbarDestroy
_internal B32 ZINK_InitToolbar(ZINK_Renderer *renderer_handle, ZINK_Toolbar *toolbar, String8 *tools_list, U32 size)
{
  toolbar->tools = malloc(sizeof(ZINK_Button) * size);
  toolbar->capacity = size;
  toolbar->count = 1;
  
  _local I32 prev_pos = 0;
  _local I32 prev_gap = 0;
  for (I32 i = 0; i < size; i++)
  {
    String8 image_path = tools_list[i];
    SDL_Surface *surface = IMG_Load(image_path);
    if (!surface)
    {
      SDL_Log("SDL_Error: %s\n", SDL_GetError());
      return false;
    }
  
    toolbar->tools[i].texture = SDL_CreateTextureFromSurface(renderer_handle->renderer, surface);
    if (!toolbar->tools[i].texture)
    {
      SDL_Log("SDL_Error: %s\n", SDL_GetError());
      return false;
    }
    SDL_DestroySurface(surface);

    toolbar->tools[i].dest.x = 20;
    toolbar->tools[i].dest.y = 20 + prev_pos + prev_gap;
    toolbar->tools[i].dest.w = BUTTON_SIZE;
    toolbar->tools[i].dest.h = BUTTON_SIZE;
    prev_pos = toolbar->tools[i].dest.y;
    prev_gap = BUTTON_GAP;

    toolbar->tools[i].active = false;
    toolbar->count += 1;
  }

  return true;
}

_internal void ZINK_UpdateToolbar(ZINK_Renderer *renderer_handle, ZINK_Toolbar *toolbar)
{
  for (U32 i = 0; i < toolbar->capacity; i++)
	  SDL_RenderTexture(renderer_handle->renderer, toolbar->tools[i].texture, NULL, &toolbar->tools[i].dest);
}

_internal B32 ZINK_InitRenderer(ZINK_Renderer *renderer_handle, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag)
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

_internal B32 ZINK_InitContext(ZINK_Renderer *renderer_handle, ZINK_Context *context)
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

_internal void ZINK_DestroyContext(ZINK_Context *context)
{
  SDL_DestroySurface(context->surface);
  SDL_DestroyTexture(context->texture);
}

_internal void ZINK_DestroyRenderer(ZINK_Renderer *renderer_handle)
{
  if (renderer_handle->initialized)
  {
    SDL_DestroyRenderer(renderer_handle->renderer);
    SDL_DestroyWindow(renderer_handle->window);    
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
