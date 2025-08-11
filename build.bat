@echo off
call "%SystemDrive%\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > NUL 2>&1

:: --- Build Type -----------------------
set debug=1
set release=0
set msvc=1
set clang=0

:: --- Pre Configuration ---------------- 
if not exist debug mkdir debug
if not exist assets mkdir assets

:: --- Unpack Arguments ------------------
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]

:: --- Flags ----------------------------
:: /F4194304 --> add this to the cl_general to restrict MAX_STACK allocation 
set cl_general= /nologo /ZI /FC
set cl_debug=   /Fd:debug\ /Fo:debug\\

:: --- Include & Libs -------------------
set cl_include= /I".\include"
set cl_link=    /link /libpath:".\lib"
set cl_lib=     User32.lib Shell32.lib Gdi32.lib SDL3.lib SDL3_Image.lib 
set cl_exclude=

:: --- Source & Output ------------------
set cl_source=  src\win32_main.c ^
								src\zink_renderer.c
set cl_output=  /Fe:debug\zink.exe

:: --- Compile --------------------------
if "%msvc%"=="1" call cl %cl_output% %cl_source% %cl_general% %cl_debug% %cl_include% %cl_link% %cl_lib% %cl_exclude%

:: --- Post Configuration ---------------
echo Copying DLL...
set dll_source=.\lib
set dll_destination=.\debug
copy /Y "%dll_source%\*.dll" "%dll_destination%" > NUL
