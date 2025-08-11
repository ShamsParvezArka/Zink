#include "types.h"
#include "../include/SDL3/SDL.h"
#include "../include/SDL_image.h"

#define ZINC_VSYNC_ENABLE  1
#define ZINC_VSYNC_DISABLE 0

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

void ZINK_TriggerMainLoop(I32 width, I32 height, String8 title);

_internal B32  ZINK_RendererInit(ZINK_Renderer *renderer_handle, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag);
_internal B32  ZINK_ContextInit(ZINK_Renderer *renderer_handle, ZINK_Context *context);
_internal void ZINK_RendererDestroy(ZINK_Renderer *renderer_handle);
_internal void ZINK_ContextDestroy(ZINK_Context *context);
_internal void ZINK_ManageInputEvent(ZINK_Context *context, SDL_Event event);
_internal F32  ZINK_LinearInterpolate(F32 point_a, F32 point_b, F32 factor);
_internal F32  ZINK_Clamp(F32 value, F32 min, F32 max);
