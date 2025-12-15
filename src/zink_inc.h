#ifndef ZINK_INC_H
#define ZINK_INC_H

#include "zink_base.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <shellapi.h>
	#include <fileapi.h>
	#include "zink_win32_platform.h"
  #include "zink_win32_resource.h"
#elif __linux__
  #include "zink_linux_platform.h"
#endif

#include "SDL3/sdl.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_image.h"
#include "SDL3/SDL_ttf.h"

#include "zink_renderer.h"
#include "zink_utils.h"





#endif // ZINK_INC_H
