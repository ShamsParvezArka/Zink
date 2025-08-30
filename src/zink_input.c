#include "zink_core.h"

_internal void
ZINK_UpdateInputState(ZINK_InputState *input)
{
  memset(input->mouse_down,     0, sizeof(input->mouse_down[0]) * 5);
  memset(input->mouse_released, 0, sizeof(input->mouse_released[0]) * 5);
  memset(input->key_down,       0, sizeof(input->key_down[0]) * SDL_SCANCODE_COUNT);
  memset(input->key_released,   0, sizeof(input->key_released[0]) * SDL_SCANCODE_COUNT);
  input->wheel_delta = 0.0f;
  
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_EVENT_QUIT:
      {
        running = false;
      } break;
      
      case SDL_EVENT_KEY_DOWN:
      {
        SDL_Keycode key = event.key.key;
        if (key == SDLK_ESCAPE) running = false;

        if (key == SDLK_W) KeyRegister(input->key_down, input->key_released, SDLK_W);
        if (key == SDLK_D) KeyRegister(input->key_down, input->key_released, SDLK_D);
        if (key == SDLK_E) KeyRegister(input->key_down, input->key_released, SDLK_E);
        if (key == SDLK_R) KeyRegister(input->key_down, input->key_released, SDLK_R);
      } break;

      case SDL_EVENT_KEY_UP:
      {
        SDL_Keycode key = event.key.key;
        if (key == SDLK_W) KeyUnregister(input->key_down, input->key_released, SDLK_W);
        if (key == SDLK_D) KeyUnregister(input->key_down, input->key_released, SDLK_D);
        if (key == SDLK_E) KeyUnregister(input->key_down, input->key_released, SDLK_E);
        if (key == SDLK_R) KeyUnregister(input->key_down, input->key_released, SDLK_R);        
      } break;      

      case SDL_EVENT_MOUSE_WHEEL:
      {
        input->wheel_delta = event.wheel.y;
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          input->mouse_drag = true;
          KeyRegister(input->mouse_down, input->mouse_released, event.button.button);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          input->mouse_drag = false;
          KeyUnregister(input->mouse_down, input->mouse_released, event.button.button);
        }
      } break;

      case SDL_EVENT_MOUSE_MOTION:
      {
        input->mouse_x = event.motion.x;
        input->mouse_y = event.motion.y;
      } break;

      default:
      {
      } break;
    }
  }
}
