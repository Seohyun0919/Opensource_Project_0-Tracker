@echo off
rem 0:Tracker build script.
rem   Double-click build.bat      -> build tracker.exe, then run it
rem   build.bat                   -> build only (from a terminal)
rem   build.bat run               -> build, then run
rem Uses MSVC (cl) if available, otherwise MinGW (gcc).
rem If src\record.c exists it is built in automatically with HAVE_RECORD.
setlocal
cd /d "%~dp0"

set DBLCLICK=0
echo %cmdcmdline% | find /i "%~nx0" >nul && set DBLCLICK=1

set SRC=src\main.c src\timer.c src\detect.c src\study_ui.c src\warning.c src\level_ui.c
set CLDEF=
set GCCDEF=

if exist src\record.c (
    set SRC=%SRC% src\record.c
    set CLDEF=/DHAVE_RECORD
    set GCCDEF=-DHAVE_RECORD
)

where cl >nul 2>nul
if not errorlevel 1 goto USE_CL
where gcc >nul 2>nul
if not errorlevel 1 goto USE_GCC

echo [ERROR] No compiler found.
echo   - Open "Developer Command Prompt for VS" and run build.bat again, or
echo   - Install MinGW-w64 (gcc) and add it to PATH.
goto FAIL_END

:USE_CL
echo [BUILD] cl (MSVC)
cl /nologo /utf-8 /W3 %CLDEF% /Fe:tracker.exe %SRC% user32.lib
if errorlevel 1 goto FAIL
goto DONE

:USE_GCC
echo [BUILD] gcc (MinGW)
gcc -g -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 %GCCDEF% -o tracker.exe %SRC% -luser32
if errorlevel 1 goto FAIL
goto DONE

:FAIL
echo [FAILED] Build failed. Check the errors above.
:FAIL_END
if "%DBLCLICK%"=="1" pause
exit /b 1

:DONE
echo [OK] tracker.exe
if "%DBLCLICK%"=="1" goto RUN
if /i "%~1"=="run" goto RUN
exit /b 0

:RUN
tracker.exe
if "%DBLCLICK%"=="1" pause
exit /b 0