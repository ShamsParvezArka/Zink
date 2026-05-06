@echo off

REM ========== Grab Compiler ==============
call "%SystemDrive%\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > NUL 2>&1

REM ========== Build Type =================
set debug=1
set release=0

REM ========== Bootstrap ==================
if not exist debug mkdir debug
if not exist release mkdir release

REM ========== Unpack Arguments ===========
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]

REM ========== Flags ======================
set cl_common=  /nologo /std:c11 /FC /Z7
set cl_debug=   /Od /Ob1 /DZINK_DEBUG_MODE=1
set cl_release= /O2 
set cl_include= /I".\include"
set cl_link=    /link /libpath:".\lib"
set cl_lib= User32.lib Shell32.lib Gdi32.lib SDL3.lib SDL3_Image.lib 

REM ========== RC Config ============
set rc_general= /nologo
set rc_source=  src\zink_icon.rc
set rc_output=  resource\logo.res

REM ========== RC Compile ============
call rc %rc_general% /fo %rc_output% %rc_source%

REM ========== Source & Output ============
set output=
if "%debug%"=="1"   set output=debug
if "%release%"=="1" set output=release

set dll_src=.\lib\*.dll
set cl_source=src\zink_main.c %rc_output%
set cl_output= /Fo:%output%\ /Fe:%output%\main.exe

REM ========== Compile =====================
if "%debug%"=="1"   call cl %cl_source% %cl_output% %cl_common% %cl_debug% %cl_include% %cl_link% %cl_lib%
if "%release%"=="1" call cl %cl_source% %cl_output% %cl_common% %cl_release% %cl_include% %cl_link% %cl_lib%

REM ========== Post Configuration ==========
copy /Y "%dll_src%" "%output%\" > NUL
