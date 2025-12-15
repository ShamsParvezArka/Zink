_global B32 running;

//~ TODO: remove these global variables  --------------------------------------------------------------------------------
_global F32 drag_start_mouse_x;
_global F32 drag_start_mouse_y;
_global F32 drag_start_target_x;
_global F32 drag_start_target_y;

//~ TODO: group them together --------------------------------------------------------------------------------
_global I32 brush_size = 4.0f;

void
ZINK_TriggerMainLoop(I32 width, I32 height, String8 title, String8 image_path)
{
  ZINK_Context zink_context = {};
  String8 driver = "direct3d11";
	DeferScope(ZINK_InitContext(&zink_context, width, height, title, driver, true, image_path), ZINK_DestroyContext(&zink_context))
	{
		// TODO(arka): should I implement this!? --------------------------------------------------------------------------------
		// ZINK_Toolbar toolbar = {};
		// U32 total_tools = 3;
		// String8 tool_path[100] = {"..\\assets\\draw.png",
		//                           "..\\assets\\eraser.png",
		//                           "..\\assets\\move.png"};
  
		//ZINK_InitToolbar(&renderer, &toolbar, tool_path, total_tools);

		ZINK_InputState input = {};
		running = true;
		while (running)
		{
			F32 delta_time = ZINK_GetDeltaTime();
			ZINK_UpdateAndRender(&zink_context, &input, delta_time);   
		}
	}
  SDL_Quit();
}

_internal B32
ZINK_InitContext(ZINK_Context *context, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag, String8 image_path)
{
	//~ NOTE: Window Setup --------------------------------------------------------------------------------
  context->window_width = width;
  context->window_height = height;
  context->window_title = title;
  context->driver = driver;
  context->vsync = vsync_flag;
  context->initialized = true;
  
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }

	if (!TTF_Init())
	{
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;		
	}
  
  I32 window_flags = SDL_WINDOW_INPUT_FOCUS |
                     SDL_WINDOW_MOUSE_FOCUS |
                     SDL_WINDOW_BORDERLESS;
  context->window = SDL_CreateWindow(context->window_title,
																		 context->window_width,
																		 context->window_height,
																		 window_flags);
  if (!context->window)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_ShowCursor();
  if (!SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT))) return false;
  if (!SDL_SetWindowFocusable(context->window, true)) return false;  
  if (!SDL_RaiseWindow(context->window)) return false;

  context->renderer = SDL_CreateRenderer(context->window, context->driver);
  if (!context->renderer)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  if (vsync_flag) SDL_SetRenderVSync(context->renderer, ZINC_VSYNC_ENABLE);
  else            SDL_SetRenderVSync(context->renderer, ZINC_VSYNC_DISABLE);

	//~ NOTE: Context, Surface, Texture Setup --------------------------------------------------------------------------------
  context->surface = IMG_Load(image_path);
  if (!context->surface)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  
  context->texture = SDL_CreateTextureFromSurface(context->renderer,
                                                  context->surface);
  if (!context->texture)
  {
    SDL_Log("SDL_Error: %s\n", SDL_GetError());    
    return false;
  }

  SDL_GetTextureSize(context->texture,
                     &context->texture_width,
                     &context->texture_height);
  
  context->dest.w = context->window_width;
  context->dest.h = context->window_height;

  context->camera.offset.x = context->window_width * 0.5f;
  context->camera.offset.y = context->window_height * 0.5f;
  context->camera.target.x = context->texture_width * 0.5f;
  context->camera.target.y = context->texture_height * 0.5f;
  context->camera.rotation = 0.0f;
  context->camera.zoom = 1.0f;
  context->camera.zoom_target = 1.0f;

	//~ NOTE: Custom Font --------------------------------------------------------------------------------
#if 0
	String8 font_path = "..\\assets\\georgia.ttf";
	context->font.size = 24;
	context->font.font = TTF_OpenFont(font_path, context->font.size);
	if (!context->font.font)
	{
		SDL_Log("SDL_Error: %s\n", SDL_GetError());    
		return false;			
	}
	context->font.color.r = 255;
	context->font.color.g = 255;
	context->font.color.b = 255;
	context->font.color.a = 255;

	context->font.surface = TTF_RenderText_Solid(context->font.font, "Hello, World!", 13, context->font.color);
	context->font.texture = SDL_CreateTextureFromSurface(context->renderer, context->font.surface);
#endif
	
  return true;
}

_internal void
ZINK_UpdateAndRender(ZINK_Context *context, ZINK_InputState *input, F32 delta_time)
{
	ZINK_UpdateInputState(input);

	ZINK_UpdateCamera(&context->camera,
										input,
										context->texture_width, context->texture_width,
										delta_time);	
  context->dest.x = context->camera.offset.x - context->camera.target.x * context->camera.zoom;
  context->dest.y = context->camera.offset.y - context->camera.target.y * context->camera.zoom;    
  context->dest.w = context->texture_width * context->camera.zoom;
  context->dest.h = context->texture_height * context->camera.zoom;	

	//~ NOTE(Rendering Layer): Background Color --------------------------------------------------------------------------------
  SDL_SetRenderDrawColor(context->renderer, 29, 29, 29, 255);
  SDL_RenderClear(context->renderer);

	//~ NOTE(Rendering Layer): Texture --------------------------------------------------------------------------------
	SDL_RenderTexture(context->renderer, context->texture, NULL, &context->dest);

	//~ NOTE(Rendering Layer): Drawing --------------------------------------------------------------------------------
	SDL_SetRenderTarget(context->renderer, context->texture);
	SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 150);

	if (input->key_down[SDLK_F]) brush_size++;	
	if (input->key_down[SDLK_D]) brush_size--;
	
	ZINK_DrawCircle(context->renderer, input->mouse_x, input->mouse_y, brush_size);
	SDL_SetRenderTarget(context->renderer, NULL);		
	
	//~ NOTE(Rendering Layer): Button, Font etc --------------------------------------------------------------------------------
	SDL_SetRenderTarget(context->renderer, context->texture);

	SDL_RenderDebugTextFormat(context->renderer, 5, 5, "Brush Size: %d", brush_size);
	SDL_RenderDebugTextFormat(context->renderer, 5, 15, "Mouse Drag: %d", input->mouse_drag);	

  SDL_RenderPresent(context->renderer);
}

_internal void
ZINK_DestroyContext(ZINK_Context *context)
{
  if (!context->initialized) return;

	SDL_DestroyRenderer(context->renderer);
	SDL_DestroyWindow(context->window);
	SDL_DestroySurface(context->surface);
	SDL_DestroyTexture(context->texture);

	// TTF_CloseFont(context->font.font);
	// SDL_DestroySurface(context->font.surface);
	// SDL_DestroyTexture(context->font.texture);	
}

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

      // KEYBOARD EVENTS --------------------------------------------------------------------------------
      case SDL_EVENT_KEY_DOWN:
      {
        SDL_Keycode key = event.key.key;
        if (key == SDLK_ESCAPE) running = false;

        if (key == SDLK_D) { KeyRegister(input->key_down, input->key_released, SDLK_D); }
        if (key == SDLK_F) { KeyRegister(input->key_down, input->key_released, SDLK_F); }				
        if (key == SDLK_R) { KeyRegister(input->key_down, input->key_released, SDLK_R); } 
      } break;

      case SDL_EVENT_KEY_UP:
      {
        SDL_Keycode key = event.key.key;
        if (key == SDLK_D) { KeyUnregister(input->key_down, input->key_released, SDLK_D); }
        if (key == SDLK_F) { KeyUnregister(input->key_down, input->key_released, SDLK_F); }								
        if (key == SDLK_R) { KeyUnregister(input->key_down, input->key_released, SDLK_R); }
      } break;      

      // MOUSE EVENTS --------------------------------------------------------------------------------
      case SDL_EVENT_MOUSE_WHEEL:
      {
        input->wheel_delta = event.wheel.y;
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          input->mouse_drag = true;
          KeyRegister(input->mouse_down, input->mouse_released, SDL_BUTTON_RIGHT);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          input->mouse_drag = false;
          KeyUnregister(input->mouse_down, input->mouse_released, SDL_BUTTON_RIGHT);
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

_internal void
ZINK_UpdateCamera(ZINK_Camera2D *cam, ZINK_InputState *input, F32 texture_width, F32 texture_height, F32 delta_time)
{
  input->world_x = (input->mouse_x - cam->offset.x) / cam->zoom + cam->target.x;
  input->world_y = (input->mouse_y - cam->offset.y) / cam->zoom + cam->target.y;
  
  if (input->mouse_down[SDL_BUTTON_RIGHT])
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

  if (input->key_down[SDLK_R])
  {
    ZINK_ResetCamera(cam, texture_width, texture_height);
  }  

  cam->zoom = ZINK_LinearInterpolate(cam->zoom, cam->zoom_target, 8.0f * delta_time);
}

_internal void
ZINK_ResetCamera(ZINK_Camera2D *cam, F32 texture_width, F32 texture_height)
{
  cam->offset.x    = texture_width * 0.5f;
  cam->offset.y    = texture_height * 0.5f;    
  cam->target.x    = texture_width * 0.5f;
  cam->target.y    = texture_height * 0.5f;
  cam->zoom        = 1.0f;
  cam->zoom_target = 1.0f;  
}
