#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK   kbd_hook_callback(int hook_code, WPARAM w_param, LPARAM l_param);
void               take_screenshot(HDC screen, int width, int height);
int                export_bitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename);
void               canvas(int width, int height, const char *title);

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE instance_prev, PSTR args, int show_cmd)
{
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

LRESULT CALLBACK kbd_hook_callback(int hook_code, WPARAM w_param, LPARAM l_param)
{
    KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)l_param;

    if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
    {
       switch (key->vkCode)
       {
       case VK_ESCAPE:
           exit(EXIT_SUCCESS);
           break;

       case 'S':
           if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
           {
               HDC screen = GetDC(NULL);
		       int width = GetDeviceCaps(screen, DESKTOPHORZRES);
		       int height = GetDeviceCaps(screen, DESKTOPVERTRES);
               const char *title = "Paint.gg";

               take_screenshot(screen, width, height);
               canvas(width, height);
           }
           break;
           
       default:
           break;
       }
    }
    
    return CallNextHookEx(NULL, hook_code, w_param, l_param);
}

void take_screenshot(HDC screen, int width, int height)
{
	HDC context = CreateCompatibleDC(screen);
    HBITMAP bmp_handle = CreateCompatibleBitmap(screen, width, height);

    SelectObject(context, bmp_handle);
    
    BOOL sig = BitBlt(context, 0, 0, width, height, screen, 0, 0, SRCCOPY);
    if (sig != 0)
    {
        LPCSTR filename = "screenshot.bmp";
	    export_bitmap(bmp_handle, context, filename);        
    }
    else
    {
        printf("ERROR: Failed to allocate bit block\n");
    }

    DeleteObject(bmp_handle);
    DeleteDC(context);
    ReleaseDC(NULL, screen);
}

int export_bitmap(HBITMAP bitmap_handle, HDC context, LPCSTR filename)
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

    DWORD bmp_size = ((bmp.bmWidth * bmp_info_header.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    HANDLE dbi = GlobalAlloc(GHND, bmp_size);
    char *lpbitmap = (char*)GlobalLock(dbi);

    BOOL sig = GetDIBits(context,
                         bitmap_handle,
                         0,
                         (UINT)bmp.bmHeight,
                         lpbitmap,
                         (BITMAPINFO*)&bmp_info_header,
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
