#ifndef WIN32_ZINK_H
#define ZINK_WIN32_PLATFORM_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include "zink_win32_resource.h"
#include "zink_core.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

typedef struct Win32_Context Win32_Context;
struct Win32_Context
{
  WNDCLASS window_class;
  HWND tray_handle;
  HMENU tray_menu;
  NOTIFYICONDATA tray_data;  
  MSG message;    
};

typedef struct Win32_State Win32_State;
struct Win32_State
{
  B32 running;
  B32 zink_mode;
};

I32 WINAPI Win32Main(HINSTANCE instance, HINSTANCE instance_previous, PWSTR command_line, int show_code);

_internal void Win32TakeScreenshot(HDC screen, I32 width, I32 height);
_internal int ExportBitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename);
_internal LRESULT CALLBACK Win32_GlobalHookCallback(I32 hook_code, WPARAM w_param, LPARAM l_param);
_internal LRESULT CALLBACK Win32MainWindowCallback(HWND window_handle, U32 message, WPARAM w_param, LPARAM l_param);

#endif // ZINK_WIN32_PLATFORM_H
