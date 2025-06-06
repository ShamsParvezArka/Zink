#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

LRESULT CALLBACK GlobalHookCallback(I32 hook_code, WPARAM w_param, LPARAM l_param);
LRESULT CALLBACK TrayCallback(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);
void             TakeCustomScreenshot(HDC screen, I32 width, I32 height);
int              ExportBitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename);
void             RenderCanvas(I32 width, I32 height, I32 true_width, I32 true_height);
void             DestroyCanvas(void);

NOTIFYICONDATA nid = {0};
HMENU tray_menu    = NULL;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE instance_prev, PSTR args, I32 show_cmd)
{
  WNDCLASSA window_class = {0};
  window_class.lpfnWndProc = TrayCallback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "Paint.gg";
  RegisterClassA(&window_class);

  HWND tray_handle = CreateWindowExA(0,
                                     "Paint.gg",
                                     "Tray Window",
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     0,
                                     0,
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
  
  HHOOK global_hook = SetWindowsHookExA(WH_KEYBOARD_LL , &GlobalHookCallback, 0, 0);

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

LRESULT CALLBACK TrayCallback(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
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

