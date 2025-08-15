#include "zink_includes.h"

#ifdef _WIN32
  #include "zink_win32_platform.c"
#elif __linux__
  #include "zink_linux_platform.c"
#endif
#include "zink_utils.c"
#include "zink_renderer.c"
#include "zink_input.c"
#include "zink_camera.c"
#include "zink_toolbar.c"

#ifdef _WIN32
I32 WINAPI
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR cmd_line, I32 show_code)
{
  return Win32Main(instance, prev_instance, cmd_line, show_code);
}

#elif __linux__
I32
main(I32 argc, String8 *argv)
{
  /*
  NOTE: No implementaiton for linux yet. If you want then feel free to
  fork this repository;
  */
  return 0;
}
#endif
