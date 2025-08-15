#include "zink_core.h"

_internal B32
ZINK_InitToolbar(ZINK_Renderer *renderer_handle, ZINK_Toolbar *toolbar, String8 *tools_list, U32 size)
{
  toolbar->tools = malloc(sizeof(ZINK_Button) * size);
  toolbar->capacity = size;
  toolbar->count = 1;
  
  _local I32 prev_pos = 0;
  _local I32 prev_gap = 0;
  for (U32 tool = 0; tool < size; tool++)
  {
    String8 image_path = tools_list[tool];
    SDL_Surface *surface = IMG_Load(image_path);
    if (!surface)
    {
      SDL_Log("SDL_Error: %s\n", SDL_GetError());
      return false;
    }
  
    toolbar->tools[tool].texture = SDL_CreateTextureFromSurface(renderer_handle->renderer, surface);
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
ZINK_UpdateToolbar(ZINK_Renderer *renderer_handle, ZINK_Toolbar *toolbar)
{
  for (U32 i = 0; i < toolbar->capacity; i++)
    SDL_RenderTexture(renderer_handle->renderer, toolbar->tools[i].texture, NULL, &toolbar->tools[i].dest);
}

_internal void
ZINK_DestroyToolbar(ZINK_Toolbar *toolbar)
{
  for (U32 tool = 0; tool < toolbar->capacity; tool++)
  {
    SDL_DestroyTexture(toolbar->tools[tool].texture);
  }
  free(toolbar->tools);
}
