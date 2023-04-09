@echo off

set CXX=g++
set CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

set RED=[0;31m
set GREEN=[0;32m
set CYAN=[0;36m
set NC=[0m

set BUILD_DIR=build

if not exist %BUILD_DIR% (
  mkdir %BUILD_DIR%
)

echo.
echo %CYAN%Generating build files...%NC%
echo.
cd %BUILD_DIR%
cmake ..
if %errorlevel% neq 0 (
  echo %RED%Error: CMake failed to generate build files.%NC%
  pause
  exit /b 1
)

echo.
echo %CYAN%Building project...%NC%
echo.
cmake --build .
if %errorlevel% neq 0 (
  echo %RED%Error: Build failed.%NC%
  pause
  exit /b 1
)

echo.
echo %GREEN%---------------------------
echo Build complete. Running renderer
echo ---------------------------%NC%
echo.
cd ..
renderer.exe