@echo off

del /s /q %~dp0\build
mkdir %~dp0\build
cd /d %~dp0\build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../src
nmake
cd %~dp0
