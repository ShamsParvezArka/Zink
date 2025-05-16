#include "renderer.conf.h"

#define KEY_COMBO_UNDO IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)
#define KEY_COMBO_REDO IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)

typedef struct History History;
struct History
{
  size_t index;
  size_t top;
  size_t count;
  RenderTexture2D *buffers;
};

History HistoryInit(size_t size);
void    HistoryPush(History *h, RenderTexture2D main_buffer);
void    HistoryUndo(History *h, RenderTexture2D *main_buffer);
void    HistoryRedo(History *h, RenderTexture2D *main_buffer);
void    TriggerZoom(Camera2D *cam, float *zoom_target, float wheel_move, Vector2 mouse_pos);
void    RenderCanvas(int width, int height, int true_width, int true_height);
void    DestroyCanvas(void);
void    DrawWithLinearInterpolation(float spacing, Color color, Vector2 cmw, Vector2 lmw);
