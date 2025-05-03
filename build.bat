@echo off
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > NUL 2>&1

set cl_general= /nologo /Zi
set cl_include= /I".\include"
set cl_link=    /link /libpath:".\lib" ".\lib\raylib.lib"   
set cl_lib=     opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib msvcrt.lib shell32.lib
set cl_exclude= /NODEFAULTLIB:libcmt
set cl_target=  kbd_hook.c canvas.c
set cl_output=  /Fe:Paintgg.exe

call cl %cl_output% %cl_target% %cl_general% %cl_include% %cl_link% %cl_lib% %cl_exclude%


