#ifndef ZINK_RENDERER_H
#define ZINK_RENDERER_H

#define ZINC_VSYNC_ENABLE  1
#define ZINC_VSYNC_DISABLE 0

#define SDL_Require(expr)                               \
  do                                                    \
  {                                                     \
    if (!(expr))                                        \
    {                                                   \
      SDL_Log("Require failer: %s\nSDL_Error: %s\n",    \
              Stringify(expr), SDL_GetError());         \
      assert((expr));                                   \
    }                                                   \
  } while (0)

#define KeyboardDown(sc, in)                                            \
  do                                                                    \
  {                                                                     \
    (in)->kbd_down[(sc)] = true;                                        \
    if (!(in)->kbd_pressed[(sc)]) { (in)->kbd_pressed[(sc)] = true; }   \
  } while (0)

#define KeyboardUp(sc, in)                      \
  do                                            \
  {                                             \
    (in)->kbd_down[(sc)] = false;               \
    (in)->kbd_released[(sc)] = true;            \
  } while (0)

#define MouseDown(btn, in)                                              \
  do                                                                    \
  {                                                                     \
    (in)->mouse_down[(btn)] = true;                                     \
    if (!(in)->mouse_pressed[(btn)]) { (in)->mouse_pressed[(btn)] = true; } \
  } while (0)

#define MouseUp(btn, in)                        \
  do                                            \
  {                                             \
    (in)->mouse_down[(btn)] = false;            \
    (in)->mouse_released[(btn)] = true;         \
  } while (0)

typedef enum ZINK_BrushMode ZINK_BrushMode;
enum ZINK_BrushMode
{
  DRAW  = 0,
  ERASE = 1
};

typedef struct ZINK_DebugFont ZINK_DebugFont;
struct ZINK_DebugFont
{
  TTF_Font *font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Color color;
  i32 size;
};

typedef struct ZINK_Camera2D ZINK_Camera2D;
struct ZINK_Camera2D
{
  SDL_FPoint offset;
  SDL_FPoint target;
  
  f32 rotation;
  f32 zoom;
  f32 zoom_target;
};

typedef struct ZINK_Context ZINK_Context;
struct ZINK_Context
{
  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Texture *sprite;

  SDL_FRect dest;

  ZINK_Camera2D camera;

  u8 *window_title;
  u8 *driver;

  u64 fps;

  u32 *original_pixels;
        
  i32 window_width;
  i32 window_height;
        
  f32 texture_width;
  f32 texture_height;

  b32 initialized;
  b32 vsync;
};

typedef struct ZINK_Button ZINK_Button;
struct ZINK_Button
{
  SDL_Texture *texture;
  SDL_FRect dest;
  
  b32 active;
};

typedef struct ZINK_InputState ZINK_InputState;
struct ZINK_InputState
{
  ZINK_BrushMode brush_mode;

  f32 world_x;
  f32 world_y;
  f32 last_world_x;
  f32 last_world_y;
  
  f32 mouse_x;
  f32 mouse_y;
  f32 wheel;

  b32 kbd_down[SDL_SCANCODE_COUNT];
  b32 kbd_pressed[SDL_SCANCODE_COUNT];
  b32 kbd_released[SDL_SCANCODE_COUNT];

  b32 mouse_down[5];
  b32 mouse_pressed[5];
  b32 mouse_released[5];
  
  b32 drag;
  u8 ascii_char;
};

typedef struct ZINK_Toolbar ZINK_Toolbar;
struct ZINK_Toolbar
{
  i32 sprite_row;
  SDL_Scancode key;
  i32 slot;
};


void ZINK_TriggerMainLoop(i32 width, i32 height, u8 *title, u8 *image_path);

internal b32  ZINK_InitContext(ZINK_Context *ctx, i32 width, i32 height, u8 *title, u8 *driver, b32 vsync_flag, u8 *image_path);
internal void ZINK_UpdateAndRender(ZINK_Context *ctx, ZINK_InputState *input, f32 delta_time);
internal void ZINK_UpdateInputState(ZINK_InputState *input);
internal void ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, f32 texture_width, f32 texture_height, f32 delta_time);
internal void ZINK_ResetCamera(ZINK_Camera2D *cam, f32 texture_width, f32 texture_height);
internal void ZINK_DestroyContext(ZINK_Context *ctx);

internal void ZINK_DispatchEvent(SDL_Event *event, ZINK_InputState *input);
internal void ZINK_ResetInputState(ZINK_InputState *input);
internal void ZINK_ProcessInput(ZINK_Context *ctx, ZINK_InputState *input, f32 delta_time);
internal void ZINK_StrokeOnCanvas(ZINK_Context *ctx, ZINK_InputState *input);
internal void ZINK_RenderToolbar(ZINK_Context *ctx, ZINK_InputState *input);

#endif // ZINK_RENDERER_H
