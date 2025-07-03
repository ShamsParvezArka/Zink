#include "types.h"

const ConfigFlags canvas_properties = (FLAG_VSYNC_HINT |
                                       FLAG_WINDOW_TOPMOST |
                                       FLAG_WINDOW_RESIZABLE |
                                       FLAG_WINDOW_UNDECORATED);
const I32 init_pos_x    = 0;
const I32 init_pos_y    = 0;
const F32 zoom_speed    = 8.0f;
const F32 zoom_min      = 0.7f;
const F32 zoom_max      = 64.0f;
const F32 brush_size    = 0.5f;
const F32 fading_factor = 0.4f;
