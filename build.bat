@echo off
setlocal

if "%CC%"=="" (
    echo Environment variable CC is not set
    exit /b 1
) else (
    set COMPILER=%CC%
)

echo Build system ^(nob.c^) is bootstrapping using %COMPILER%
%COMPILER% nob.c -o nob.exe

if errorlevel 1 (
    echo unable to build nob.c
    exit /b 1
)

nob.exe %*
