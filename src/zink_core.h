#ifndef ZINK_RENDERER_H
#define ZINK_RENDERER_H

#include "zink_includes.h"

// renderer --------------------------------------------------------------
#define ZINC_VSYNC_ENABLE  1
#define ZINC_VSYNC_DISABLE 0
// ui --------------------------------------------------------------------
#define BUTTON_SIZE 30
#define BUTTON_GAP 10

// input -----------------------------------------------------------------
#define KeyRegister(down, released, key)        \
  do                                            \
  {                                             \
    down[key]     = true;                       \
    released[key] = false;                      \
  } while (0);
#define KeyUnregister(down, released, key)      \
  do                                            \
  {                                             \
    down[key]     = false;                      \
    released[key] = true;                       \
  } while (0);  

typedef struct ZINK_Renderer ZINK_Renderer;
struct ZINK_Renderer
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  
  I32 window_width;
  I32 window_height;

  B32 initialized;
  B32 vsync;

  String8 window_title;
  String8 driver;  
};

typedef struct ZINK_Camera2D ZINK_Camera2D;
struct ZINK_Camera2D
{
  SDL_FPoint offset;
  SDL_FPoint target;
  
  F32 rotation;
  F32 zoom;
  F32 zoom_target;
};

typedef struct ZINK_Context ZINK_Context;
struct ZINK_Context
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_FRect dest;

  ZINK_Camera2D camera;
  
  F32 texture_width;
  F32 texture_height;
};

typedef struct ZINK_Button ZINK_Button;
struct ZINK_Button
{
  SDL_Texture *texture;
  SDL_FRect dest;
  
  B32 active;
};

typedef struct ZINK_Toolbar ZINK_Toolbar;
struct ZINK_Toolbar
{
  ZINK_Button *tools;
  U32 capacity;  
  U32 count;
};

typedef struct ZINK_InputState ZINK_InputState;
struct ZINK_InputState
{
  F32 mouse_x;
  F32 mouse_y;
  F32 world_x;
  F32 world_y;
  F32 wheel_delta;

  B32 mouse_drag;
  B32 mouse_down[5];
  B32 mouse_released[5];
  B32 key_down[SDL_SCANCODE_COUNT];
  B32 key_released[SDL_SCANCODE_COUNT];
};

void ZINK_TriggerMainLoop(I32 width, I32 height, String8 title, String8 path);

_internal B32  ZINK_InitRenderer(ZINK_Renderer *renderer, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag);
_internal B32  ZINK_InitContext(ZINK_Renderer *renderer, ZINK_Context *context);
_internal B32  ZINK_InitToolbar(ZINK_Renderer *renderer, ZINK_Toolbar *toolbar, String8 *toolbar_list, U32 toolbar_size);

_internal void ZINK_UpdateToolbar(ZINK_Renderer *renderer, ZINK_Toolbar *toolbar);
_internal void ZINK_UpdateAndRender(ZINK_Renderer *renderer, ZINK_Context *context, F32 delta_time);
_internal void ZINK_UpdateInputState(ZINK_InputState *input);
_internal void ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, F32 texture_width, F32 texture_height, F32 delta_time);

_internal void ZINK_DestroyRenderer(ZINK_Renderer *renderer);
_internal void ZINK_DestroyContext(ZINK_Context *context);
_internal void ZINK_DestroyToolbar(ZINK_Toolbar *toolbar);

_internal void ZINK_ResetCamera(ZINK_Camera2D *cam, F32 texture_width, F32 texture_height);

_internal F32  ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor);
_internal F32  ZINK_Clamp(F32 value, F32 min, F32 max);
_internal F32  ZINK_GetDeltaTime();

#endif // ZINK_RENDERER_H
