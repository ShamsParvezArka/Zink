#include <windows.h>
#include <winuser.h>
#include "types.h"
#include "../include/SDL3/SDL.h"
#include "../include/SDL_image.h"
#include "zink_renderer.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

NOTIFYICONDATA tray_icon_data = {};
HMENU tray_menu = NULL;

_global B32 running;
_global B32 zink_mode;

_internal void Win32TakeScreenshot(HDC screen, I32 width, I32 height)
{
	HDC context = CreateCompatibleDC(screen);
  HBITMAP bmp_handle = CreateCompatibleBitmap(screen, width, height);

  SelectObject(context, bmp_handle);

  BOOL sig = BitBlt(context, 0, 0, width, height, screen, 0, 0, SRCCOPY);
  if (sig != 0)
  {
    LPCSTR filename = "..\\assets\\screenshot.bmp";
    ExportBitmap(bmp_handle, context, filename);        
  }
  else
  {
    printf("ERROR: Failed to allocate bit block\n");
  }

  DeleteObject(bmp_handle);
  DeleteDC(context);
}

_internal int ExportBitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename)
{
  BITMAP bmp;
  GetObject(bitmap_handle, sizeof(BITMAP), &bmp);

  BITMAPINFOHEADER bmp_info_header = {};
  bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
  bmp_info_header.biWidth = bmp.bmWidth;
  bmp_info_header.biHeight = -bmp.bmHeight;  
  bmp_info_header.biPlanes = 1;
  bmp_info_header.biBitCount = 32;
  bmp_info_header.biCompression = BI_RGB;
  bmp_info_header.biSizeImage = 0;
  bmp_info_header.biXPelsPerMeter = 0;
  bmp_info_header.biYPelsPerMeter = 0;
  bmp_info_header.biClrUsed = 0;
  bmp_info_header.biClrImportant = 0;

  I32 height = abs(bmp_info_header.biHeight);
  DWORD bmp_size = ((bmp.bmWidth * bmp_info_header.biBitCount + 31) / 32) * 4 * height;
  HANDLE dbi = GlobalAlloc(GHND, bmp_size);
  char *lpbitmap = (char*)GlobalLock(dbi);

  BOOL sig = GetDIBits(context,
                       bitmap_handle,
                       0,
                       (UINT)bmp.bmHeight,
                       lpbitmap,
                       (BITMAPINFO *)&bmp_info_header,
                       DIB_RGB_COLORS);
	if (sig == 0)
  {
    printf("ERROR: Failed to retrieve data from bitmap\n");
    return 0; 
  }

  HANDLE file_handle = CreateFileA(filename,
                                   GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
  DWORD dbi_size = bmp_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  BITMAPFILEHEADER bmp_file_header = {};
  bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmp_file_header.bfSize = dbi_size;
  bmp_file_header.bfType = 0x4D42;

  // TODO(arka): implement proper error handling
  DWORD bytes_written;
  WriteFile(file_handle, &bmp_file_header, sizeof(BITMAPFILEHEADER), &bytes_written, NULL);
  WriteFile(file_handle, &bmp_info_header, sizeof(BITMAPINFOHEADER), &bytes_written, NULL);
  WriteFile(file_handle, lpbitmap, bmp_size, &bytes_written, NULL);

  GlobalUnlock(dbi);
  GlobalFree(dbi);
  CloseHandle(file_handle);

  return 1;
}


LRESULT CALLBACK Win32_GlobalHookCallback(I32 hook_code, WPARAM w_param, LPARAM l_param)
{
  KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)l_param;

  if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
  {
    switch (key->vkCode)
    {
      //case VK_ESCAPE:
      //{
      //  zink_mode = false;
      //} break;

      case 'D':
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && !zink_mode)
        {
          HDC screen = GetDC(NULL);
          I32 width = GetDeviceCaps(screen, DESKTOPHORZRES);
          I32 height = GetDeviceCaps(screen, DESKTOPVERTRES);          
          Win32TakeScreenshot(screen, width, height);
          zink_mode = true;
        } break;

      default:
      {
      } break;
    }
  }
    
  return CallNextHookEx(NULL, hook_code, w_param, l_param);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window_handle, U32 message, WPARAM w_param, LPARAM l_param)
{
  LRESULT result = 0;
  
  switch (message)
  {
	  case WM_TRAYICON:
    {
      if (l_param == WM_RBUTTONUP)
      {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(window_handle);
        TrackPopupMenu(tray_menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, window_handle, NULL);
        PostMessage(window_handle, WM_NULL, 0, 0);
      }
    } break;

    case WM_COMMAND:
    {
      if (LOWORD(w_param) == ID_TRAY_EXIT)
      {
        PostQuitMessage(0);
      }
    } break;

    case WM_CLOSE:
    {
      DestroyWindow(window_handle);
    } break;

    case WM_DESTROY:
    {
      Shell_NotifyIcon(NIM_DELETE, &tray_icon_data);    
      PostQuitMessage(0);
    } break;

    default:
    {
      result = DefWindowProc(window_handle, message, w_param, l_param);
    } break;
  }

  return result;
}

I32 WINAPI wWinMain(HINSTANCE instance, HINSTANCE instance_previous, PWSTR command_line, int show_code)
{
  WNDCLASS window_class = {};
  
  window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  window_class.lpfnWndProc = Win32MainWindowCallback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "ZINK_Class";

	HHOOK _global_hook = SetWindowsHookExA(WH_KEYBOARD_LL , &Win32_GlobalHookCallback, 0, 0);

  if (RegisterClass(&window_class))
  {
    HWND tray_handle = CreateWindowExA(0,
                                   "ZINK_Class",
                                   NULL,
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   HWND_MESSAGE,
                                   NULL,
                                   instance,
                                   NULL);
    tray_menu = CreatePopupMenu();
    AppendMenuA(tray_menu, MF_STRING, ID_TRAY_EXIT, "Exit");

    tray_icon_data.cbSize = sizeof(tray_icon_data);
    tray_icon_data.hWnd = tray_handle;
    tray_icon_data.uID = 1;
    tray_icon_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tray_icon_data.uCallbackMessage = WM_TRAYICON;
    tray_icon_data.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(tray_icon_data.szTip, "ZINK_");
    Shell_NotifyIcon(NIM_ADD, &tray_icon_data);    

    running = true;

    while (running)
    {
      MSG message;        
      while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
      {
        if (message.message == WM_QUIT)
        {
          running = false;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
      
      if (zink_mode)
      {
        String8 title = "ZINK";
#if 1
        HDC screen = GetDC(NULL);        
        I32 width = GetDeviceCaps(screen, DESKTOPHORZRES);
        I32 height = GetDeviceCaps(screen, DESKTOPVERTRES);
#endif
        ZINK_TriggerMainLoop(width, height, title);
        zink_mode = false;
      }
    }
  }
  UnhookWindowsHookEx(_global_hook);

  return 0;
}

