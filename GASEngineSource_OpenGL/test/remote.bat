@echo off
echo.
echo Remote Desktop will be temporarily disconnected . . . please reconnect after a few seconds.
echo.
@echo on
REM The active session has an arrow as the first character
setlocal EnableDelayedExpansion
FOR /F %%A in ('qwinsta') do (
	set tempSessionName=%%A
	if "!tempSessionName:~0,1!"==">"  ( 
		@echo on
		tscon.exe !tempSessionName:~1! /v /dest:console 
		@echo off
	)	
)
@echo off
echo.
cd /d D:\GasEngine_OSR\build-2\app\
echo Starting App.ArthubViewer.exe . . .
@echo on
start D:\GasEngine_OSR\build-2\app\Debug\App.ArthubViewer.exe -input D:\render_test\GASEngineV2\converted\girlwalk\girlwalk.fbx -token=333151e00019 -draw_bbox=false -play_speed=1.0
@echo off
echo.
pause