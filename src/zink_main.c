#include "zink_inc.h"
#include "zink_inc.c"

#ifdef _WIN32
i32 WINAPI
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR cmd_line, i32 show_code)
{
  return Win32EntryPoint(instance, prev_instance, cmd_line, show_code);
}

#elif __linux__
i32
main(i32 argc, u8 *argv)
{
  /* 
  ** NOTE: No implementaiton for linux yet. If you want then feel free to
  ** fork this repository;
  */
  return 0;
}
#endif
