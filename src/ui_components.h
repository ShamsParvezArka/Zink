#define BUTTON_SIZE 30
#define BUTTON_GAP 3

typedef struct Button
{
  Texture2D texture;
  Rectangle rect;
  bool state;
} Button;

typedef enum ButtonName
{
  BUTTON_MOVE,
  BUTTON_DRAW,
  BUTTON_ERASER,
} ButtonName;

