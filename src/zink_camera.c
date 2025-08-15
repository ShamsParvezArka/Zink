#include "zink_core.h"

_internal void
ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, F32 delta_time)
{
  input->world_x = (input->mouse_x - cam->offset.x) / cam->zoom + cam->target.x;
  input->world_y = (input->mouse_y - cam->offset.y) / cam->zoom + cam->target.y;
  
  if (input->mouse_down[SDL_BUTTON_LEFT])
  {
    drag_start_mouse_x = input->mouse_x;
    drag_start_mouse_y = input->mouse_y;
    drag_start_target_x = cam->target.x;
    drag_start_target_y = cam->target.y;    
  }
  
  if (input->wheel_delta)
  {
    cam->offset.x = input->mouse_x;
    cam->offset.y = input->mouse_y;
    cam->target.x = input->world_x;
    cam->target.y = input->world_y;
    
    _local F32 zoom_factor = 1.1f;
    if (input->wheel_delta > 0) cam->zoom_target *= zoom_factor;
    if (input->wheel_delta < 0) cam->zoom_target /= zoom_factor;
    
    cam->zoom_target = ZINK_Clamp(cam->zoom_target, 0.5f, 7.0f);    
  }
  
  if (input->mouse_drag)
  {
    F32 dx = (input->mouse_x - drag_start_mouse_x) / cam->zoom;
    F32 dy = (input->mouse_y - drag_start_mouse_y) / cam->zoom;
    cam->target.x = drag_start_target_x - dx;
    cam->target.y = drag_start_target_y - dy;    
  }

  cam->zoom = ZINK_LinearInterpolate(cam->zoom, cam->zoom_target, 8.0f * delta_time);  
}
