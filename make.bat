@echo off

set CXX=g++
set CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

set RED=[0;31m
set GREEN=[0;32m
set YELLOW=[0;33m
set NC=[0m

set BUILD_DIR=build

if not exist %BUILD_DIR% (
  mkdir %BUILD_DIR%
)

echo %YELLOW%Generating build files...%NC%
cd %BUILD_DIR%
cmake ..
if %errorlevel% neq 0 (
  echo %RED%Error: CMake failed to generate build files.%NC%
  exit /b 1
)

echo %YELLOW%Building project...%NC%
cmake --build .
if %errorlevel% neq 0 (
  echo %RED%Error: Build failed.%NC%
  exit /b 1
)

echo %GREEN%---------------------------
echo Build complete. Running renderer
echo ---------------------------%NC%
echo.
cd ..
renderer.exe