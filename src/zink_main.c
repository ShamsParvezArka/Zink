#include "zink_includes.h"

#include "zink_win32_platform.c"
#include "zink_renderer.c"

#ifdef _WIN32
I32 WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR cmd_line, int show_code) {
  return Win32Main(instance, prev_instance, cmd_line, show_code);
}
#elif __linux__
I32 main(I32 argc, String8 *argv)
{
  /*
  NOTE: No implementaiton for linux yet. If you want then please fork this
        repository;
  */
  return 0;
}
#endif
