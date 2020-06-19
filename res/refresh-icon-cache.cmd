@ECHO OFF
CHCP 65001
SETLOCAL ENABLEEXTENSIONS
SET script_directory=%~dp0

WHERE /Q ie4uinit ^
        || ECHO The ie4uinit executable not found. ^
        && CALL :pause_if_double_click ^
        && EXIT /B 1

:: Windows 10
ie4uinit -Show ^
        || ECHO Refresh icon cache failed. ^
        && CALL :pause_if_double_click ^
        && EXIT /B 2

:: Older Windows
ie4uinit -ClearIconCache ^
        || ECHO Refresh icon cache failed. ^
        && CALL :pause_if_double_click ^
        && EXIT /B 3

EXIT /B 0

:pause_if_double_click
        ECHO %CMDCMDLINE% | FINDSTR /L %COMSPEC% 1>NUL 2>NUL ^
                && PAUSE
        EXIT /B 0
