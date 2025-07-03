#include <stdlib.h>
#include "../include/raylib.h"
#include "../include/rlgl.h"
#include "../include/raymath.h"
#include "renderer.h"
#include "types.h"
#include "ui_components.h"

void RenderCanvas(I32 width, I32 height, I32 true_width, I32 true_height)
{
  SetConfigFlags(canvas_properties);
	InitWindow(width, height, "Paint.gg");
  SetWindowPosition(init_pos_x, init_pos_y);
  SetWindowFocused();

  Image img = LoadImage("screenshot.bmp");
  Texture2D tex = LoadTextureFromImage(img);
  UnloadImage(img);
  SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);

  F32 zoom_target = 1.0f;
  Camera2D camera = {.zoom = 1.0f};

  Rectangle src  = {0.0f, 0.0f, (F32)tex.width, (F32)tex.height};
  Rectangle dst  = {0.0f, 0.0f, true_width, true_height};
  V2 origin = {0, 0};

  History undo_history = HistoryInit((U64)20);
  I32 foo = 0;
  RenderTexture2D main_buffer = LoadRenderTexture(true_width, true_height);
  V2 last_mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);

  //TODO(arka): make this more organized
  Image img_move = LoadImage("../assets/move.png");
	Button btn_move = {0};
  btn_move.texture = LoadTextureFromImage(img_move);
  btn_move.rect = (Rectangle){20, 20, BUTTON_SIZE, BUTTON_SIZE};
  btn_move.state = false;
  UnloadImage(img_move);
  
  Image img_draw = LoadImage("../assets/draw.png");
  Button btn_draw = {0};
  btn_draw.texture = LoadTextureFromImage(img_draw);
  btn_draw.rect = (Rectangle){20, btn_move.rect.y + BUTTON_SIZE + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE};
  btn_draw.state = false;
  UnloadImage(img_draw);

  Image img_eraser = LoadImage("../assets/eraser.png");
  Button btn_eraser = {0};
  btn_eraser.texture = LoadTextureFromImage(img_eraser);
  btn_eraser.rect = (Rectangle){20, btn_draw.rect.y + BUTTON_SIZE + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE};
  btn_eraser.state = false;
  UnloadImage(img_eraser);

  Button *buttons = malloc(sizeof(Button) * 3);
  buttons[BUTTON_MOVE] = btn_move;
  buttons[BUTTON_DRAW] = btn_draw;
  buttons[BUTTON_ERASER] = btn_eraser;

  Rectangle toolbar_area = {buttons[0].rect.x,
  													buttons[0].rect.y,
                            BUTTON_SIZE,
                            buttons[0].rect.y + buttons[2].rect.y + buttons[2].rect.height};
  I32 current_button = BUTTON_MOVE;
  
  while (!WindowShouldClose())
  {
    F32 delta = GetFrameTime();
    F32 wheel_move = GetMouseWheelMove();
    V2 mouse_delta = GetMouseDelta();
    V2 mouse_pos   = GetMousePosition();

    I32 e = GetKeyPressed();
    switch (e)
    {
    case KEY_B:
      current_button = BUTTON_DRAW;
      break;
    case KEY_M:
      current_button = BUTTON_MOVE;
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

    if (IsKeyPressed(KEY_ESCAPE))
    {
      if (IsWindowState(FLAG_WINDOW_UNDECORATED))
      {
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        break;
      }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && current_button == BUTTON_MOVE)
    {
      mouse_delta = Vector2Scale(mouse_delta, -1.0f/camera.zoom);
      camera.target = Vector2Add(camera.target, mouse_delta);
    }
    if (GetMouseWheelMove != 0)
    {
      TriggerZoom(&camera, &zoom_target, wheel_move, mouse_pos);        
    }
    
    V2 current_mouse_world = GetScreenToWorld2D(mouse_pos, camera);
    BeginTextureMode(main_buffer);
    {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
          current_button == BUTTON_DRAW &&
          !CheckCollisionPointRec(GetMousePosition(), toolbar_area))
      {
        DrawWithLinearInterpolation(brush_size, RED, current_mouse_world, last_mouse_world);
      }
      else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && current_button == BUTTON_DRAW)
      {
        HistoryPush(&undo_history, main_buffer);          
      }
    }
    EndTextureMode();

    last_mouse_world = current_mouse_world;
    camera.zoom = Lerp(camera.zoom, zoom_target, zoom_speed * delta);

		//TODO(arka): make this more organized    
    for (I32 i = 0; i < 3; i++)
    {
      if (CheckCollisionPointRec(GetMousePosition(), buttons[i].rect) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
      {
        if (i == BUTTON_MOVE)
        {
          current_button = BUTTON_MOVE;
        }
        else if (i == BUTTON_DRAW)
        {
          current_button = BUTTON_DRAW;
        }
        else if (i == BUTTON_ERASER)
        {
          current_button = BUTTON_ERASER;          
        }
      }      
    }
    
    BeginDrawing();
    {
      ClearBackground((Color){13, 13, 13});
      BeginMode2D(camera);
      {
        DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        Rectangle src = {0,
                         0,
                         main_buffer.texture.width,
                         -main_buffer.texture.height};
        V2 pos = {0, 0};
        DrawTextureRec(main_buffer.texture, src, pos, WHITE);
      }
      EndMode2D();

      //TODO(arka): wrap this chunk into a callback function
      for (I32 i = 0; i < 3; i++)
      {
        if (i == current_button)
        {
          DrawRectangleRounded(buttons[i].rect, 0.2f, 100, Fade(RAYWHITE, fading_factor + 0.3));                    
        }
        else
        {
          DrawRectangleRounded(buttons[i].rect, 0.2f, 100, Fade(RAYWHITE, fading_factor));          
        }
        DrawTexture(buttons[i].texture, buttons[i].rect.x, buttons[i].rect.y, WHITE);
      }
      
    }
    EndDrawing();
  }
  UnloadTexture(tex);
  UnloadRenderTexture(main_buffer);
  CloseWindow();
}

History HistoryInit(U64 size)
{
  History h = {.count = 0,
               .capacity = size,
               .buffers = malloc(sizeof(*(h.buffers)) * size)};

  return h;
}

void HistoryPush(History *h, RenderTexture2D buffer)
{
  RenderTexture2D back_buffer = LoadRenderTexture(buffer.texture.width,
                                                  buffer.texture.height);
  BeginTextureMode(back_buffer);
  {
		Rectangle dimension = {0, 0, buffer.texture.width, -buffer.texture.height};
    V2 position  = {0, 0};
    DrawTextureRec(buffer.texture, dimension, position, WHITE);
  }
  EndTextureMode();
  
  h->buffers[h->count] = back_buffer;
  h->count++;
  memset(h->buffers + h->count, 0, h->capacity);
}

void HistoryUndo(History *h, RenderTexture2D *buffer)
{
  if (h->count > 0)
  {
    h->count--;
  }
  *buffer = h->buffers[h->count];
}

void HistoryRedo(History *h, RenderTexture2D *buffer)
{
  if (h->count < h->capacity - 1)
  {
    h->count++;
  }
  *buffer = h->buffers[h->count];
}

void TriggerZoom(Camera2D *camera, F32 *zoom_target, F32 wheel_move, V2 mouse_pos)
{
  V2 mouse_world_pos = GetScreenToWorld2D(mouse_pos, *camera);
  F32 zoom_factor = 1.0f + (wheel_move * 0.1f);
            
  *zoom_target = Clamp(*zoom_target * zoom_factor, zoom_min, zoom_max);
  camera->offset = mouse_pos;
  camera->target = mouse_world_pos;
}

void DestroyCanvas(void)
{
  CloseWindow();
}

void DrawWithLinearInterpolation(F32 spacing, Color color, V2 cmw, V2 lmw)
{
  V2 dt = Vector2Subtract(cmw, lmw);
  F32 length = Vector2Length(dt);

  if (length > 0.0f)
  {
    V2 direction = Vector2Scale(dt, 1.0f / length);
    for (F32 i = 0; i <= length; i += spacing)
    {
      V2 point = Vector2Add(lmw, Vector2Scale(direction, i));
      DrawCircleV(point, 2.0f, color);
    }
  }
}
