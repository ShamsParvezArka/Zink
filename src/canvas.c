#include "../include/raylib.h"
#include "../include/rlgl.h"
#include "../include/raymath.h"
#include <stdlib.h>

#define SMOOTH_ZOOM_ENABLE 1
#define DRAWING_MODE_ENABLE 1

const float zoom_speed = 8.0f;
const float zoom_min   = 0.7f;
const float zoom_max   = 64.0f;
bool drawing_mode = false;

typedef struct History
{
  size_t index;
  size_t top;
  size_t count;
  RenderTexture2D *buffers;
} History;

History HistoryInit(size_t size)
{
  History h = {};
  h.index = 0;
  h.top   = 0;
  h.count = 0;
  h.buffers = malloc(sizeof(RenderTexture2D) * size);

  return h;
}

void HistoryPush(History *h, RenderTexture2D main_buffer)
{
  RenderTexture2D back_buffer = LoadRenderTexture(main_buffer.texture.width,
                                                  main_buffer.texture.height);
  BeginTextureMode(back_buffer);
  {
		Rectangle dimension = {0, 0, main_buffer.texture.width, -main_buffer.texture.height};
    Vector2   position  = {0, 0};
    DrawTextureRec(main_buffer.texture, dimension, position, WHITE);
  }
  EndTextureMode();
  
  h->buffers[h->index] = back_buffer;
  h->top = h->index;
  h->index++;
  h->count++;
}

void HistoryUndo(History *h, RenderTexture2D *main_buffer)
{
  if (h->top > 0) {h->top--;}
  *main_buffer = h->buffers[h->top];
}

void HistoryRedo(History *h, RenderTexture2D *main_buffer)
{
  if (h->top < h->count - 1) {h->top++;}
  *main_buffer = h->buffers[h->top];
}

void TriggerZoom(Camera2D *camera, float *zoom_target, float wheel_move, Vector2 mouse_pos)
{
  Vector2 mouse_world_pos = GetScreenToWorld2D(mouse_pos, *camera);
  float zoom_factor = 1.0f + (wheel_move * 0.1f);
            
  *zoom_target = Clamp(*zoom_target * zoom_factor, zoom_min, zoom_max);
  camera->offset = mouse_pos;
  camera->target = mouse_world_pos;
}

void RenderCanvas(int width, int height, int true_width, int true_height)
{
  SetConfigFlags(FLAG_VSYNC_HINT |
                 FLAG_WINDOW_TOPMOST |
                 FLAG_WINDOW_RESIZABLE |
                 FLAG_WINDOW_UNDECORATED);
  
	InitWindow(width, height, "Paint.gg");
  SetWindowPosition(0, 0);

  Image img = LoadImage("screenshot.bmp");
  Texture2D tex = LoadTextureFromImage(img);
  SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
  UnloadImage(img);

  float zoom_target = 1.0f;
  Camera2D camera = {.zoom = 1.0f};

  Rectangle src  = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
  Rectangle dst  = {0.0f, 0.0f, true_width, true_height};
  Vector2 origin = {0, 0};

  History undo_history = HistoryInit((size_t)20);
  RenderTexture2D main_buffer = LoadRenderTexture(true_width, true_height);
  Vector2 last_mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);

  while (!WindowShouldClose())
  {
    float delta         = GetFrameTime();
    Vector2 mouse_delta = GetMouseDelta();
    Vector2 mouse_pos   = GetMousePosition();
    float wheel_move    = GetMouseWheelMove();

    if (IsKeyPressed(KEY_B)) {drawing_mode = !drawing_mode;}
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z))
    {
			HistoryUndo(&undo_history, &main_buffer);
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y))
    {
			HistoryRedo(&undo_history, &main_buffer);
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !drawing_mode)
    {
      mouse_delta = Vector2Scale(mouse_delta, -1.0f/camera.zoom);
      camera.target = Vector2Add(camera.target, mouse_delta);
    }

    #ifdef SMOOTH_ZOOM_ENABLE
      camera.zoom = Lerp(camera.zoom, zoom_target, zoom_speed * delta);
    #endif
    if (GetMouseWheelMove() != 0)
    {
      TriggerZoom(&camera, &zoom_target, wheel_move, mouse_pos);
    }

    Vector2 current_mouse_world = GetScreenToWorld2D(mouse_pos, camera);
    BeginTextureMode(main_buffer);
    {
			#ifdef DRAWING_MODE_ENABLE
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && drawing_mode)
      {
        float spacing = 0.5f;
        Vector2 dt = Vector2Subtract(current_mouse_world,
                                     last_mouse_world);
        float length = Vector2Length(dt);

        if (length > 0.0f)
        {
          Vector2 direction = Vector2Scale(dt, 1.0f / length);
          for (float i = 0; i <= length; i += spacing)
          {
            Vector2 point = Vector2Add(last_mouse_world,
                                       Vector2Scale(direction, i));
            DrawCircleV(point, 2.0f, RED);
          }
        }
      }

      if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && drawing_mode)
      {
        HistoryPush(&undo_history, main_buffer);
      }
      #endif
    }
    EndTextureMode();
    
    last_mouse_world = current_mouse_world;
         
    BeginDrawing();
    {
      ClearBackground((Color){13, 13, 13});
      BeginMode2D(camera);
      {
        DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        DrawTextureRec(main_buffer.texture,
                       (Rectangle){0,
                                   0,
                                   main_buffer.texture.width,
                                   -main_buffer.texture.height},
                       (Vector2){0, 0},
                       WHITE);
      }
      EndMode2D();
    }
    EndDrawing();
  }
  UnloadTexture(tex);
  UnloadRenderTexture(main_buffer);
}

void DestroyCanvas(void)
{
  CloseWindow();
}
