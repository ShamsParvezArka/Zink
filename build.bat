@echo off
call "%SystemDrive%\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > NUL 2>&1

:: ============= Pre-Configuration =============
if not exist debug mkdir debug
if not exist assets mkdir assets

:: ============= Flags =============
:: /F4194304 --> add this to the cl_general to restrict MAX_STACK allocation 
set cl_general= /nologo /ZI /FC /F4194304
set cl_debug=   /Fd:debug\ /Fo:debug\\

:: ============= Include & Libs =============
set cl_include= /I".\include"
set cl_link=    /link /libpath:".\lib"
set cl_lib=      SDL3.lib User32.lib Gdi32.lib SDL3_Image.lib Shell32.lib
set cl_exclude=

:: ============= Source & Output =============
set cl_target=  src\win32_main.c ^
								src\zink_renderer.c
set cl_output=  /Fe:debug\zink.exe

:: ============= Compile =============
call cl %cl_output% %cl_target% %cl_general% %cl_debug% %cl_include% %cl_link% %cl_lib% %cl_exclude%
