@echo off
rd /s /q "%~dp0/.xmake"
rd /s /q "%~dp0/build"
rd /s /q "%~dp0/vsxmake2026"
xmake f -m releasedbg
xmake package
xcopy /s "%~dp0/build/packages" "%~dp0"
rd /s /q "%~dp0/.xmake"
rd /s /q "%~dp0/build"
