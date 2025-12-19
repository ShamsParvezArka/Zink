#ifndef ZINK_RENDERER_H
#define ZINK_RENDERER_H

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

#define KeyRegisterOnce(pressed, key)						\
	do																						\
	{																							\
		pressed[key] = true;												\
	} while(0)																		\

typedef struct ZINK_DebugFont ZINK_DebugFont;
struct ZINK_DebugFont
{
	TTF_Font *font;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Color color;
	I32 size;
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
  SDL_Window *window;
  SDL_Renderer *renderer;

	SDL_Surface *surface;
  SDL_Texture *texture;

  SDL_FRect dest;

  ZINK_Camera2D camera;
//	ZINK_DebugFont font;

	String8 window_title;
  String8 driver;
  
  I32 window_width;
  I32 window_height;
	
  F32 texture_width;
  F32 texture_height;

  B32 initialized;
  B32 vsync;
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
	B32 mouse_pressed[5];
  B32 mouse_down[5];
  B32 mouse_released[5];
  B32 key_down[SDL_SCANCODE_COUNT];
  B32 key_released[SDL_SCANCODE_COUNT];
};

void ZINK_TriggerMainLoop(I32 width, I32 height, String8 title, String8 image_path);

_internal B32  ZINK_InitContext(ZINK_Context *context, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag, String8 image_path);
_internal B32  ZINK_InitInputState(ZINK_InputState *input);
_internal void ZINK_UpdateAndRender(ZINK_Context *context, ZINK_InputState *input, F32 delta_time);
_internal void ZINK_UpdateInputState(ZINK_InputState *input);
_internal void ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, F32 texture_width, F32 texture_height, F32 delta_time);
_internal void ZINK_ResetCamera(ZINK_Camera2D *cam, F32 texture_width, F32 texture_height);

_internal void ZINK_DestroyContext(ZINK_Context *context);

#endif // ZINK_RENDERER_H
