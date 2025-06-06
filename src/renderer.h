#include "renderer.conf.h"
#include "types.h"

#define KEY_COMBO_UNDO IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)
#define KEY_COMBO_REDO IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)

typedef struct History
{
  U64 count;
  U64 capacity;
  RenderTexture2D *buffers;
} History;

History HistoryInit(U64 size);
void    HistoryPush(History *h, RenderTexture2D buffer);
void    HistoryUndo(History *h, RenderTexture2D *buffer);
void    HistoryRedo(History *h, RenderTexture2D *buffer);
void    TriggerZoom(Camera2D *cam, float *zoom_target, float wheel_move, Vector2 mouse_pos);
void    RenderCanvas(I32 width, I32 height, I32 true_width, I32 true_height);
void    DestroyCanvas(void);
void    DrawWithLinearInterpolation(float spacing, Color color, Vector2 cmw, Vector2 lmw);
