@echo off
REM Launch the GUI with Qt6 in PATH
set PATH=C:\Qt\Qt6\6.10.1\mingw_64\bin;C:\Qt\Qt6\Tools\mingw1310_64\bin;%PATH%
start "" "%~dp0build-mingw\gui\scenario_viewer_gui.exe"
