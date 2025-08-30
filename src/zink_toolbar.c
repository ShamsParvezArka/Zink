#include "zink_core.h"

_internal B32
ZINK_InitToolbar(ZINK_Renderer *renderer, ZINK_Toolbar *toolbar, String8 *tools_list, U32 size)
{
  toolbar->tools = malloc(sizeof(ZINK_Button) * size);
  toolbar->capacity = size;
  toolbar->count = 1;
  
  _local I32 prev_pos = 0;
  _local I32 prev_gap = 0;
  for EachIndex(tool, size)
  {
    String8 image_path = tools_list[tool];
    SDL_Surface *surface = IMG_Load(image_path);
    if (!surface)
    {
      SDL_Log("SDL_Error: %s\n", SDL_GetError());
      return false;
    }
  
    toolbar->tools[tool].texture = SDL_CreateTextureFromSurface(renderer->renderer, surface);
    if (!toolbar->tools[tool].texture)
    {
      SDL_Log("SDL_Error: %s\n", SDL_GetError());
      return false;
    }
    SDL_DestroySurface(surface);

    toolbar->tools[tool].dest.x = 20;
    toolbar->tools[tool].dest.y = 20 + prev_pos + prev_gap;
    toolbar->tools[tool].dest.w = BUTTON_SIZE;
    toolbar->tools[tool].dest.h = BUTTON_SIZE;
    prev_pos = toolbar->tools[tool].dest.y;
    prev_gap = BUTTON_GAP;

    toolbar->tools[tool].active = false;
    toolbar->count += 1;
  }

  return true;
}

_internal void
ZINK_UpdateToolbar(ZINK_Renderer *renderer, ZINK_Toolbar *toolbar)
{
  for EachIndex(idx, toolbar->capacity)  
    SDL_RenderTexture(renderer->renderer, toolbar->tools[idx].texture, NULL, &toolbar->tools[idx].dest);
}

_internal void
ZINK_DestroyToolbar(ZINK_Toolbar *toolbar)
{
  for EachIndex(tool, toolbar->capacity)
  {
    SDL_DestroyTexture(toolbar->tools[tool].texture);
  }
  free(toolbar->tools);
}
