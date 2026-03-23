#ifndef ZINK_WIN32_PLATFORM_H
#define ZINK_WIN32_PLATFORM_H

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
  b32 running;
  b32 zink_mode;
};

i32 WINAPI Win32EntryPoint(HINSTANCE instance, HINSTANCE instance_previous, PWSTR command_line, int show_code);

internal void Win32TakeScreenshot(HDC screen, i32 width, i32 height);
internal int ExportBitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename);
internal LRESULT CALLBACK Win32GlobalHookCallback(i32 hook_code, WPARAM w_param, LPARAM l_param);
internal LRESULT CALLBACK Win32MainWindowCallback(HWND window_handle, u32 message, WPARAM w_param, LPARAM l_param);

#endif // ZINK_WIN32_PLATFORM_H
