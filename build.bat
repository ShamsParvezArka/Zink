@echo off
call "%SystemDrive%\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > NUL 2>&1

:: --- Build Type ---------------------------------------------------------
set debug=0
set release=1
set msvc=1
set clang=0

:: --- Pre Configuration --------------------------------------------------
if not exist debug mkdir debug
if not exist assets mkdir assets

:: --- Unpack Arguments ----------------------------------------------------
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile] 

:: --- Flags --------------------------------------------------------------
:: /F4194304 --> add this to the cl_general to restrict MAX_STACK allocation 
set cl_general= /nologo /ZI /FC
set cl_debug=   /Fd:debug\ /Fo:debug\\

:: --- Include & Libs -----------------------------------------------------
set cl_include= /I".\include"
set cl_link=    /link /libpath:".\lib"
set cl_lib=     User32.lib Shell32.lib Gdi32.lib SDL3.lib SDL3_Image.lib 
set cl_exclude=

:: --- RC Config ---------------------------------------------------------
set rc_general= /nologo
set rc_source=  src\zink_icon.rc
set rc_output=  resource\logo.res

:: --- RC Compile --------------------------------------------------------
call rc %rc_general% /fo %rc_output% %rc_source%

:: --- CL Source & Output -------------------------------------------------
set cl_source=  src\zink_main.c %rc_output%
set cl_output=  /Fe:debug\zink.exe

:: --- CL Compile ---------------------------------------------------------
if "%debug%"=="1"   call cl /DZINK_DEBUG_MODE %cl_output% %cl_source% %cl_general% %cl_debug% %cl_include% %cl_link% %cl_lib% %cl_exclude%
if "%release%"=="1" call cl %cl_output% %cl_source% %cl_general% %cl_debug% %cl_include% %cl_link% %cl_lib% %cl_exclude%

:: --- Post Configuration -------------------------------------------------
echo Copying DLL...
set dll_source=.\lib
set dll_destination=.\debug
copy /Y "%dll_source%\*.dll" "%dll_destination%" > NUL
echo Done.
