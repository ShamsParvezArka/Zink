_global Win32_State state = {};
_global Win32_Context win32_context = {};

I32 WINAPI
Win32EntryPoint(HINSTANCE instance, HINSTANCE instance_previous, PWSTR command_line, int show_code)
{
#if ZINK_DEBUG_MODE
  Win32SpawnConsole();
#endif
  
  win32_context.window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  win32_context.window_class.lpfnWndProc = Win32MainWindowCallback;
  win32_context.window_class.hInstance = instance;
  win32_context.window_class.lpszClassName = "ZINK_Class";

  HHOOK global_hook = SetWindowsHookExA(WH_KEYBOARD_LL , &Win32GlobalHookCallback, 0, 0);

  if (RegisterClass(&win32_context.window_class))
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
    win32_context.tray_menu = CreatePopupMenu();
    AppendMenuA(win32_context.tray_menu, MF_STRING, ID_TRAY_EXIT, "Exit");
    win32_context.tray_data.cbSize = sizeof(win32_context.tray_data);
    win32_context.tray_data.hWnd = tray_handle;
    win32_context.tray_data.uID = 1;
    win32_context.tray_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    win32_context.tray_data.uCallbackMessage = WM_TRAYICON;
    win32_context.tray_data.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ZINK_ICON));
    strcpy(win32_context.tray_data.szTip, "ZINK");
    Shell_NotifyIcon(NIM_ADD, &win32_context.tray_data);    

    state.running = true;
    while (state.running)
    {
      if (PeekMessage(&win32_context.message, 0, 0, 0, PM_REMOVE))
      {
        if (win32_context.message.message == WM_QUIT)
        {
          state.running = false;
        }
        TranslateMessage(&win32_context.message);
        DispatchMessage(&win32_context.message);
      }
      else
      {
        WaitMessage();
      }
      
      if (state.zink_mode)
      {
        HDC screen = {};
				DeferScope(screen = GetDC(NULL), ReleaseDC(NULL, screen))
				{
					String8 title = "ZINK";										
					I32 width = GetDeviceCaps(screen, DESKTOPHORZRES);
					I32 height = GetDeviceCaps(screen, DESKTOPVERTRES);

					String8 temp_file_path = {};
					DeferScope(temp_file_path = malloc(MAX_PATH), free(temp_file_path))
					{
						GetTempPath(MAX_PATH, temp_file_path);
						strcat(temp_file_path, "zink_screenshot.bmp");
						ZINK_TriggerMainLoop(width, height, title, temp_file_path);
						state.zink_mode = false;
					}					
				}
      }
    }
  }
  UnhookWindowsHookEx(global_hook);

  return 0;
}

_internal void
Win32TakeScreenshot(HDC screen, I32 width, I32 height)
{
  HDC win32_context = CreateCompatibleDC(screen);
  HBITMAP bmp_handle = CreateCompatibleBitmap(screen, width, height);
  SelectObject(win32_context, bmp_handle);

  BOOL sig = BitBlt(win32_context, 0, 0, width, height, screen, 0, 0, SRCCOPY);
  if (sig != 0)
  {
    String8 temp_file_path = {};
    DeferScope(temp_file_path = malloc(MAX_PATH), free(temp_file_path))
    {
      GetTempPath(MAX_PATH, temp_file_path);
      strcat(temp_file_path, "zink_screenshot.bmp");

      // TODO: Bullet proof this
      Win32ExportBitmap(bmp_handle, win32_context, temp_file_path);      
    }
  }
  else
  {
    printf("ERROR: Failed to allocate bit block\n");
  }

  DeleteObject(bmp_handle);
  DeleteDC(win32_context);
}

_internal
void Win32SpawnConsole()
{
  AllocConsole();
  FILE *fp;
  freopen_s(&fp, "CONOUT$", "w", stdout);
  freopen_s(&fp, "CONOUT$", "w", stderr);
  freopen_s(&fp, "CONIN$",  "r", stdin);
}

_internal int
Win32ExportBitmap(HBITMAP bitmap_handle, HDC win32_context, LPCSTR filename)
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
  char *lpbitmap = (char *)GlobalLock(dbi);

  BOOL sig = GetDIBits(win32_context,
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

	////////////////////////////////
  // TODO: bullet proof this
	//
  DWORD bytes_written;
  WriteFile(file_handle, &bmp_file_header, sizeof(BITMAPFILEHEADER), &bytes_written, NULL);
  WriteFile(file_handle, &bmp_info_header, sizeof(BITMAPINFOHEADER), &bytes_written, NULL);
  WriteFile(file_handle, lpbitmap, bmp_size, &bytes_written, NULL);

  GlobalUnlock(dbi);
  GlobalFree(dbi);
  CloseHandle(file_handle);

  return 1;
}

_internal LRESULT CALLBACK
Win32MainWindowCallback(HWND window_handle, U32 message, WPARAM w_param, LPARAM l_param)
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
        TrackPopupMenu(win32_context.tray_menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, window_handle, NULL);
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
      Shell_NotifyIcon(NIM_DELETE, &win32_context.tray_data);
			DestroyIcon(win32_context.tray_data.hIcon);
      PostQuitMessage(0);
    } break;

    default:
    {
      result = DefWindowProc(window_handle, message, w_param, l_param);
    } break;
  }

  return result;
}

_internal LRESULT CALLBACK
Win32GlobalHookCallback(I32 hook_code, WPARAM w_param, LPARAM l_param)
{
  KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)l_param;

  if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
  {
    switch (key->vkCode)
    {
      case (VK_SPACE):
      {
        I32 ctrl = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ||
                   (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
        I32 win  = (GetAsyncKeyState(VK_LWIN) & 0x8000) ||
                   (GetAsyncKeyState(VK_RWIN) & 0x8000);
        if (ctrl && win && !state.zink_mode)
        {
          HDC screen = GetDC(NULL);
          I32 width = GetDeviceCaps(screen, DESKTOPHORZRES);
          I32 height = GetDeviceCaps(screen, DESKTOPVERTRES);          
          Win32TakeScreenshot(screen, width, height);
          state.zink_mode = true;      
        }
      } break;

      default:
      {
      } break;
    }
  }
    
  return CallNextHookEx(NULL, hook_code, w_param, l_param);
}
