#ifdef _WIN32
  #include "zink_win32_platform.c"
#elif __linux__
  #include "zink_linux_platform.c"
#endif

#include "zink_renderer.c"
#include "zink_utils.c"

