@ECHO OFF
REM 
REM Script to clean NoteCase project tree on windows (delete temporary files)
REM 	NOTE: requires some recent Windows (NT based)
REM

IF NOT "%OS%"=="Windows_NT" GOTO :End

rmdir /S /Q ..\bin\debug
rmdir /S /Q ..\bin\release
rmdir /S /Q ..\bin\profile
rmdir /S /Q ..\locale

rmdir /S /Q ..\tools\NotecaseLauncher\Debug
rmdir /S /Q ..\tools\NotecaseLauncher\Release

del ..\NoteCase.ncb
del ..\NoteCase.plg
del ..\NoteCase.opt
del ..\NoteCase.*.user
del ..\Makefile.config
del ..\res\Notecase.aps
del  /F /Q /A H ..\NoteCase.suo
del ..\po\*.mo
del ..\src\*.bak
del ..\src\lib\*.bak
del ..\src\gui\*.bak
del ..\src\_unx\*.bak
del ..\src\_win\*.bak

del ..\tools\NoteCaseLauncher\NoteCaseLauncher.aps
del ..\tools\NoteCaseLauncher\NoteCaseLauncher.plg
del ..\tools\NoteCaseLauncher\NoteCaseLauncher.ncb
del  /F /Q /A H ..\tools\NoteCaseLauncher\NoteCaseLauncher.suo
del ..\tools\NoteCaseLauncher\NoteCaseLauncher.*.user

REM Mingw files
del /Q ..\bin\*.*
REM restore dummy file in bin directory
copy ..\docs\readme.txt ..\bin\readme.txt

REM cleanup zlib subproject
rmdir /S /Q ..\src\lib\zlib\debug
rmdir /S /Q ..\src\lib\zlib\release
rmdir /S /Q ..\src\lib\zlib\profile
del ..\src\lib\zlib\zlib.ncb
del ..\src\lib\zlib\zlib.opt
del ..\src\lib\zlib\zlib_static.plg
del ..\src\lib\zlib\zlib_static.lib
del ..\src\lib\zlib_static.lib
del ..\src\lib\zlib\*.user

:End