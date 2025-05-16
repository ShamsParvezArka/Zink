#include "../include/raylib.h"
#include "../include/rlgl.h"
#include "../include/raymath.h"
#include "renderer.h"
#include <stdlib.h>

void RenderCanvas(int width, int height, int true_width, int true_height)
{
  SetConfigFlags(canvas_properties);
	InitWindow(width, height, "Paint.gg");
  SetWindowPosition(0, 0);

  Image img = LoadImage("screenshot.bmp");
  Texture2D tex = LoadTextureFromImage(img);
  UnloadImage(img);
  SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);

  float zoom_target = 1.0f;
  Camera2D camera = {.zoom = 1.0f};

  Rectangle src  = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
  Rectangle dst  = {0.0f, 0.0f, true_width, true_height};
  Vector2 origin = {0, 0};

  History undo_history = HistoryInit((size_t)20);
  RenderTexture2D main_buffer = LoadRenderTexture(true_width, true_height);
  Vector2 last_mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);

  int drawing_mode = 0;
  
  while (!WindowShouldClose())
  {
    float delta         = GetFrameTime();
    float wheel_move    = GetMouseWheelMove();
    Vector2 mouse_delta = GetMouseDelta();
    Vector2 mouse_pos   = GetMousePosition();

    int e = GetKeyPressed();
    switch (e)
    {
    case KEY_B:
      drawing_mode = !drawing_mode;
      break;
    case KEY_Z:
      if (IsKeyDown(KEY_LEFT_CONTROL))
      {
        HistoryUndo(&undo_history, &main_buffer);        
      }
      break;
    case KEY_Y:
      if (IsKeyDown(KEY_LEFT_CONTROL))
      {
        HistoryRedo(&undo_history, &main_buffer);
      }
      break;
    default:

      break;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !drawing_mode)
    {
      mouse_delta = Vector2Scale(mouse_delta, -1.0f/camera.zoom);
      camera.target = Vector2Add(camera.target, mouse_delta);
    }
    if (GetMouseWheelMove != 0)
    {
      TriggerZoom(&camera, &zoom_target, wheel_move, mouse_pos);        
    }
    camera.zoom = Lerp(camera.zoom, zoom_target, zoom_speed * delta);
    
    Vector2 current_mouse_world = GetScreenToWorld2D(mouse_pos, camera);
    BeginTextureMode(main_buffer);
    {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && drawing_mode)
      {
        DrawWithLinearInterpolation(0.5f, RED, current_mouse_world, last_mouse_world);
      }
      if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && drawing_mode)
      {
        HistoryPush(&undo_history, main_buffer);          
      }
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

History HistoryInit(size_t size)
{
  History h = {.index = 0,
               .top   = 0,
               .count = 0,
               .buffers = malloc(sizeof(*(h.buffers)) * size)};

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
  if (h->top > 0)
  {
    h->top--;
  }
  *main_buffer = h->buffers[h->top];
}

void HistoryRedo(History *h, RenderTexture2D *main_buffer)
{
  if (h->top < h->count - 1)
  {
    h->top++;
  }
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

void DestroyCanvas(void)
{
  CloseWindow();
}

void DrawWithLinearInterpolation(float spacing, Color color, Vector2 cmw, Vector2 lmw)
{
  Vector2 dt = Vector2Subtract(cmw, lmw);
  float length = Vector2Length(dt);

  if (length > 0.0f)
  {
    Vector2 direction = Vector2Scale(dt, 1.0f / length);
    for (float i = 0; i <= length; i += spacing)
    {
      Vector2 point = Vector2Add(lmw, Vector2Scale(direction, i));
      DrawCircleV(point, 2.0f, color);
    }
  }
}

