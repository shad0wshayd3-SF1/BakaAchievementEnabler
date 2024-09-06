echo off
rd /s /q "%~dp0/.xmake"
rd /s /q "%~dp0/build"
rd /s /q "%~dp0/vsxmake2022"
xmake project -k vsxmake
