#pragma once

#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK kbd_hook_callback(int hook_code, WPARAM w_param, LPARAM l_param);
LRESULT CALLBACK tray_callback(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);
void             take_screenshot(HDC screen, int width, int height);
int              export_bitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename);
void             canvas(int width, int height, int true_width, int true_height);
void             destroy_canvas(void);

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

NOTIFYICONDATA nid = {0};
HMENU tray_menu = NULL;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE instance_prev, PSTR args, int show_cmd)
{
  WNDCLASSA window_class = {0};
  window_class.lpfnWndProc = tray_callback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "Paint.gg";
  RegisterClassA(&window_class);

  HWND tray_handle = CreateWindowExA(0,
                                     "Paint.gg",
                                     "Tray Window",
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     300,
                                     200,
                                     NULL,
                                     NULL,
                                     instance,
                                     NULL);

  tray_menu = CreatePopupMenu();
  AppendMenuA(tray_menu, MF_STRING, ID_TRAY_EXIT, "Exit");

  nid.cbSize = sizeof(nid);
  nid.hWnd = tray_handle;
  nid.uID = 1;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYICON;
  nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  strcpy(nid.szTip, "Paint.gg");
  Shell_NotifyIcon(NIM_ADD, &nid);
  
  HHOOK kbd_hook = SetWindowsHookExA(WH_KEYBOARD_LL , &kbd_hook_callback, 0, 0);

  while (1)
  {
    MSG msg;
    BOOL sig = GetMessage(&msg, 0, 0, 0);
    if (sig > 0)
    {
      TranslateMessage(&msg); 
      DispatchMessage(&msg);
    }
    else
    {
      break;
    }
  }

  return 0;
}

LRESULT CALLBACK tray_callback(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
{
  switch (message)
  {
  case WM_TRAYICON:
    if (l_param == WM_RBUTTONUP)
    {
      POINT pt;
      GetCursorPos(&pt);
      SetForegroundWindow(handle);
      TrackPopupMenu(tray_menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, handle, NULL);
    }
    break;

  case WM_COMMAND:
    if (LOWORD(w_param) == ID_TRAY_EXIT)
    {
      Shell_NotifyIcon(NIM_DELETE, &nid);
      PostQuitMessage(0);
      return 0;
    }
		break;
    
  case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, &nid);
    PostQuitMessage(0);
    return 0;

  default:
    break;
  }

  return DefWindowProc(handle, message, w_param, l_param);
}
