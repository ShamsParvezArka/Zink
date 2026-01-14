_global B32 running;

// TODO: remove these global variables
_global F32 drag_start_mouse_x;
_global F32 drag_start_mouse_y;
_global F32 drag_start_target_x;
_global F32 drag_start_target_y;

_global I32 brush_size = 4.0f;

void
ZINK_TriggerMainLoop(I32 width, I32 height, String8 title, String8 image_path)
{
  ZINK_Context zink_context = {};
  String8 driver = "direct3d11";
	DeferScope(ZINK_InitContext(&zink_context, width, height, title, driver, true, image_path), ZINK_DestroyContext(&zink_context))
	{
		ZINK_InputState input = {};
		ZINK_InitInputState(&input);

		U64 last_tick = 0;
		running = true;
		while (running)
		{
			U64 current_tick = SDL_GetTicks();
			
			F32 delta_time = ZINK_GetDeltaTime();
			ZINK_UpdateAndRender(&zink_context, &input, delta_time);

			zink_context.fps++;
			if (current_tick > last_tick + 1000)
			{
				last_tick = current_tick;
				zink_context.fps = 0;
			}
		}
	}
  SDL_Quit();
}

_internal B32
ZINK_InitContext(ZINK_Context *context, I32 width, I32 height, String8 title, String8 driver, B32 vsync_flag, String8 image_path)
{
	// NOTE: Window Setup
  context->window_width = width;
  context->window_height = height;
  context->window_title = title;
  context->driver = driver;
  context->vsync = vsync_flag;
  context->initialized = true;
	context->fps = 0;
  
  Require(SDL_Init(SDL_INIT_VIDEO));
  
  I32 window_flags = SDL_WINDOW_INPUT_FOCUS |
                     SDL_WINDOW_MOUSE_FOCUS |
                     SDL_WINDOW_BORDERLESS;
  context->window = SDL_CreateWindow(context->window_title,
																		 context->window_width,
																		 context->window_height,
																		 window_flags);
  Require(context->window);

  SDL_ShowCursor();
  if (!SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT))) { return false; }
  if (!SDL_SetWindowFocusable(context->window, true))                    { return false; }
  if (!SDL_RaiseWindow(context->window))                                 { return false; }

  context->renderer = SDL_CreateRenderer(context->window, context->driver);
  Require(context->renderer);
	SDL_SetRenderDrawBlendMode(context->renderer, SDL_BLENDMODE_BLEND);	
  
  if (vsync_flag) SDL_SetRenderVSync(context->renderer, ZINC_VSYNC_ENABLE);
  else            SDL_SetRenderVSync(context->renderer, ZINC_VSYNC_DISABLE);
	
	// NOTE: Screenshot texture
	SDL_Surface *tmp;
	DeferScope(tmp = IMG_Load(image_path), SDL_DestroySurface(tmp))
	{
	  Require(tmp);
		context->surface = SDL_ConvertSurface(tmp, SDL_PIXELFORMAT_RGBA8888);
		Require(context->surface);
	}

	I32 pixel_count = context->surface->w * context->surface->h;
	context->original_pixels = (U32 *)malloc(pixel_count * sizeof(U32));
	memcpy(context->original_pixels, context->surface->pixels, pixel_count * sizeof(U32));
	
	context->texture_width  = context->surface->w;
	context->texture_height = context->surface->h;	
	context->texture = SDL_CreateTexture(context->renderer,
																			 SDL_PIXELFORMAT_RGBA8888,
																			 SDL_TEXTUREACCESS_STREAMING,
																			 context->texture_width,
																			 context->texture_height);
	Require(context->texture);
	SDL_SetTextureBlendMode(context->texture, SDL_BLENDMODE_BLEND);

	// NOTE: Copy initial image into screenshot texture
	SDL_Texture *temp;
	DeferScope(temp = SDL_CreateTextureFromSurface(context->renderer, context->surface), SDL_DestroyTexture(temp))
	{
		SDL_SetRenderTarget(context->renderer, context->texture);
		SDL_RenderTexture(context->renderer, temp, NULL, NULL);		
	}

  context->dest.w = context->window_width;
  context->dest.h = context->window_height;

	// NOTE: Sprite texture
	String8 sprite_path = "..\\assets\\keys.png";
	DeferScope(tmp = IMG_Load(sprite_path), SDL_DestroySurface(tmp))
	{
	  Require(tmp);
		context->sprite = SDL_CreateTextureFromSurface(context->renderer, tmp);
		Require(context->texture);
	}

  context->camera.offset.x = context->window_width * 0.5f;
  context->camera.offset.y = context->window_height * 0.5f;
  context->camera.target.x = context->texture_width * 0.5f;
  context->camera.target.y = context->texture_height * 0.5f;
  context->camera.rotation = 0.0f;
  context->camera.zoom = 1.0f;
  context->camera.zoom_target = 1.0f;

  return true;
}

_internal void
ZINK_UpdateAndRender(ZINK_Context *context, ZINK_InputState *input, F32 delta_time)
{
	// NOTE: Update input & camera
	ZINK_UpdateInputState(input);
	ZINK_UpdateCamera(&context->camera,
										input,
										context->texture_width, context->texture_height,
										delta_time);	
  context->dest.x = context->camera.offset.x - context->camera.target.x * context->camera.zoom;
  context->dest.y = context->camera.offset.y - context->camera.target.y * context->camera.zoom;    
  context->dest.w = context->texture_width * context->camera.zoom;
  context->dest.h = context->texture_height * context->camera.zoom;	

	// NOTE: Update brush size
	if (input->key_down[SDLK_F]) { brush_size = ZINK_Clamp(++brush_size, 4.0f, 30.0f); }
	if (input->key_down[SDLK_D]) { brush_size = ZINK_Clamp(--brush_size, 4.0f, 30.0f); }

	// NOTE:(Rendering Layer): Painting
	SDL_SetRenderTarget(context->renderer, context->texture);
	SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);

	U32 *pixels = (U32 *)context->surface->pixels;
	U32 *original_pixels = context->original_pixels;
	I32 pitch = context->surface->pitch / sizeof(U32);	

	if (input->mouse_down[SDL_BUTTON_LEFT] && input->brush_mode == ERASE)
	{
		SDL_LockSurface(context->surface);

		F32 dt_x = input->world_x - input->last_world_x;
		F32 dt_y = input->world_y - input->last_world_y;
		F32 distance = sqrtf((dt_x * dt_x) + (dt_y * dt_y));

		if (distance > 0.0f)
		{
			F32 inverse = 1.0f / distance;
			F32 direction_x = dt_x * inverse;
			F32 direction_y = dt_y * inverse;
			for (F32 idx = 0.0f; idx <= distance; idx += 0.1f)
			{
				F32 point_x = input->last_world_x + direction_x * idx;
				F32 point_y = input->last_world_y + direction_y * idx;
				ZINK_EraseCircleFilledCPU(pixels, original_pixels,
																	pitch,
																	context->texture_width, context->texture_height,
																	(I32)point_x, (I32)point_y,
																	brush_size);
			}
		}

		SDL_UnlockSurface(context->surface);
		input->last_world_x = input->world_x;
		input->last_world_y = input->world_y;		
	}

	if (input->mouse_down[SDL_BUTTON_LEFT] && input->brush_mode == DRAW)
	{
		SDL_LockSurface(context->surface);

		F32 dt_x = input->world_x - input->last_world_x;
		F32 dt_y = input->world_y - input->last_world_y;
		F32 distance = sqrtf((dt_x * dt_x) + (dt_y * dt_y));

		if (distance > 0.0f)
		{
			F32 inverse = 1.0f / distance;
			F32 direction_x = dt_x * inverse;
			F32 direction_y = dt_y * inverse;
			for (F32 idx = 0.0f; idx <= distance; idx += 0.1f)
			{
				F32 point_x = input->last_world_x + direction_x * idx;
				F32 point_y = input->last_world_y + direction_y * idx;
				ZINK_DrawCircleFilledCPU(pixels,
																 pitch,
																 context->texture_width, context->texture_height,
																 (I32)point_x, (I32)point_y,
																 brush_size);
			}
		}
		
		SDL_UnlockSurface(context->surface);
		input->last_world_x = input->world_x;
		input->last_world_y = input->world_y;
	}
	SDL_SetRenderTarget(context->renderer, NULL);
	
	// NOTE(Rendering Layer): Clear background
  SDL_SetRenderDrawColor(context->renderer, 29, 29, 29, 255);
  SDL_RenderClear(context->renderer);

	// NOTE(LAYER): Draw texture
	SDL_UpdateTexture(context->texture, NULL, context->surface->pixels, context->surface->pitch);
	SDL_RenderTexture(context->renderer, context->texture, NULL, &context->dest);

	// NOTE(Rendering Layer): Brush preview
	SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 150);	
	ZINK_DrawCircle(context->renderer, input->mouse_x, input->mouse_y, brush_size);

	// NOTE(Sprite Layer)
	// TODO(Future arka): Take care of this duct taped UI code. Please... 0.0
	const F32 sprite_scale_factor = 0.5;
	
	SDL_FRect src_btn_r = {0, 64 * 0, 64, 64};
	SDL_FRect dst_btn_r = {context->window_width - 32, context->window_height - 32,
		                     64 * sprite_scale_factor, 64 * sprite_scale_factor};
	if (input->key_down[SDLK_R])     { src_btn_r.x = 64; }
	if (input->key_released[SDLK_R]) { src_btn_r.x = 0; }
	SDL_RenderTexture(context->renderer, context->sprite, &src_btn_r, &dst_btn_r);

	SDL_FRect src_btn_e = {0, 64 * 1, 64, 64};
	SDL_FRect dst_btn_e = {context->window_width - 32, context->window_height - 32 * 2,
		                     64 * sprite_scale_factor, 64 * sprite_scale_factor};
	if (input->key_down[SDLK_E])     { src_btn_e.x = 64; }
	if (input->key_released[SDLK_E]) { src_btn_e.x = 0; }  
	SDL_RenderTexture(context->renderer, context->sprite, &src_btn_e, &dst_btn_e);

	SDL_FRect src_btn_d = {0, 64 * 2, 64, 64};
	SDL_FRect dst_btn_d = {context->window_width - 32, context->window_height - 32 * 3,
		                     64 * sprite_scale_factor, 64 * sprite_scale_factor};
	if (input->key_down[SDLK_D])     { src_btn_d.x = 64; }
	if (input->key_released[SDLK_D]) { src_btn_d.x = 0; }  
	SDL_RenderTexture(context->renderer, context->sprite, &src_btn_d, &dst_btn_d);

	SDL_FRect src_btn_f = {0, 64 * 3, 64, 64};
	SDL_FRect dst_btn_f = {context->window_width - 32, context->window_height - 32 * 4,
		                     64 * sprite_scale_factor, 64 * sprite_scale_factor};
	if (input->key_down[SDLK_F])     { src_btn_f.x = 64; }
	if (input->key_released[SDLK_F]) { src_btn_f.x = 0; }  
	SDL_RenderTexture(context->renderer, context->sprite, &src_btn_f, &dst_btn_f);

	SDL_FRect src_mouse_left  = {0, 64 * 4, 64, 64};
	SDL_FRect src_mouse_right = {0, 64 * 5, 64, 64};
	SDL_FRect src_mouse_wheel = {0, 64 * 6, 64, 64};	
	SDL_FRect dst_mouse = {context->window_width - 32, context->window_height - 32 * 5,
		                     64 * sprite_scale_factor, 64 * sprite_scale_factor};

	SDL_RenderTexture(context->renderer, context->sprite, &src_mouse_left, &dst_mouse);
	if (input->mouse_down[SDL_BUTTON_LEFT])
	{
	  src_mouse_left.x = 64;
		SDL_RenderTexture(context->renderer, context->sprite, &src_mouse_left, &dst_mouse);		
	}
	if (input->mouse_down[SDL_BUTTON_RIGHT])
	{
	  src_mouse_right.x = 64;
		SDL_RenderTexture(context->renderer, context->sprite, &src_mouse_right, &dst_mouse);		
	}
	if (input->wheel_delta != 0)
	{
	  src_mouse_wheel.x = 64;
		SDL_RenderTexture(context->renderer, context->sprite, &src_mouse_wheel, &dst_mouse);		
	}
	
	// NOTE(Rendering Layer): Button, Font etc
#if ZINK_DEBUG_MODE
	SDL_RenderDebugTextFormat(context->renderer, 5, 5,  "Brush Size: %d", brush_size);
	SDL_RenderDebugTextFormat(context->renderer, 5, 15, "Mouse Drag: %d", input->mouse_drag);
	SDL_RenderDebugTextFormat(context->renderer, 5, 25, "Mouse LClick: %d", input->mouse_down[SDL_BUTTON_LEFT]);
	SDL_RenderDebugTextFormat(context->renderer, 5, 35, "Mouse RClick: %d", input->mouse_down[SDL_BUTTON_RIGHT]);
	SDL_RenderDebugTextFormat(context->renderer, 5, 45, "Current Mouse (X, Y): (%f, %f)", input->world_x, input->world_y);
	SDL_RenderDebugTextFormat(context->renderer, 5, 55, "Last Mouse (X, Y)   : (%f, %f)", input->last_world_x, input->last_world_y);
#endif

  SDL_RenderPresent(context->renderer);
}

_internal void
ZINK_DestroyContext(ZINK_Context *context)
{
  if (!context->initialized) return;

	free(context->original_pixels);
	context->original_pixels = 0;
	
	SDL_DestroyTexture(context->texture);
	SDL_DestroySurface(context->surface);	
	SDL_DestroyRenderer(context->renderer);
	SDL_DestroyWindow(context->window);
}

_internal B32
ZINK_InitInputState(ZINK_InputState *input)
{
	input->brush_mode = DRAW;
  memset(input->mouse_down,     0, sizeof(input->mouse_down[0]) * 5);
  memset(input->mouse_released, 0, sizeof(input->mouse_released[0]) * 5);
  memset(input->key_down,       0, sizeof(input->key_down[0]) * SDL_SCANCODE_COUNT);
  memset(input->key_released,   0, sizeof(input->key_released[0]) * SDL_SCANCODE_COUNT);

	return true;
}

_internal void
ZINK_UpdateInputState(ZINK_InputState *input)
{
	memset(input->mouse_pressed,  0, sizeof(input->mouse_pressed[0]) * 5);			
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
			
			////////////////////////////////
      // NOTE: KEYBOARD EVENTS
			//
			case SDL_EVENT_KEY_DOWN:
      {
        SDL_Keycode key = event.key.key;
				
        if (key == SDLK_ESCAPE) running = false;
        if (key == SDLK_D) { KeyRegister(input->key_down, input->key_released, SDLK_D); }
        if (key == SDLK_F) { KeyRegister(input->key_down, input->key_released, SDLK_F); }				
        if (key == SDLK_R) { KeyRegister(input->key_down, input->key_released, SDLK_R); }	

				if (key == SDLK_E && input->brush_mode == DRAW)
				{
					input->brush_mode = ERASE;
					KeyRegister(input->key_down, input->key_released, SDLK_E);
				}
				else if (key == SDLK_E && input->brush_mode == ERASE)
				{
					input->brush_mode = DRAW;					
					KeyUnregister(input->key_down, input->key_released, SDLK_E);
				}
      } break;
			
      case SDL_EVENT_KEY_UP:
      {
        SDL_Keycode key = event.key.key;

        if (key == SDLK_D) { KeyUnregister(input->key_down, input->key_released, SDLK_D); }
        if (key == SDLK_F) { KeyUnregister(input->key_down, input->key_released, SDLK_F); }								
        if (key == SDLK_R) { KeyUnregister(input->key_down, input->key_released, SDLK_R); }
      } break;      

			////////////////////////////////
      // NOTE: MOUSE EVENTS
			//
      case SDL_EVENT_MOUSE_WHEEL:
      {
        input->wheel_delta = event.wheel.y;
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
				if (event.button.button == SDL_BUTTON_LEFT)
        {
					input->last_world_x = input->world_x;
					input->last_world_y = input->world_y;
          KeyRegister(input->mouse_down, input->mouse_released, SDL_BUTTON_LEFT);
        }				
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          input->mouse_drag = true;
					KeyRegisterOnce(input->mouse_pressed, SDL_BUTTON_RIGHT);
          KeyRegister(input->mouse_down, input->mouse_released, SDL_BUTTON_RIGHT);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          KeyUnregister(input->mouse_down, input->mouse_released, SDL_BUTTON_LEFT);
        }
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
  
  if (input->mouse_pressed[SDL_BUTTON_RIGHT])
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
