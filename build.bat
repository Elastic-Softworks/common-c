@echo off
REM =============================================================================
REM  COMMON-C WINDOWS BUILD SCRIPT
REM  ELASTIC SOFTWORKS 2025
REM
REM  this script provides a simple way to compile the COMMON-C library
REM  on windows systems. it assumes that a `make` utility (like the one
REM  provided by MinGW or MSYS2) is available in the system's PATH.
REM
REM  Usage:
REM    build.bat          - compiles the library in release mode.
REM    build.bat debug    - compiles the library in debug mode.
REM    build.bat clean    - removes all build artifacts.
REM    build.bat install  - (if defined in Makefile) installs the library.
REM =============================================================================

echo --- COMMON-C WINDOWS BUILD ---

REM Check if a specific target was passed (e.g., "debug", "clean")

if "%1"=="" (
    echo BUILDING IN RELEASE MODE...
    make
) else (
    echo BUILDING TARGET: %1...
    make %1
)

if %errorlevel% neq 0 (
    echo.
    echo BUILD FAILED!
    exit /b %errorlevel%
)

echo.
echo BUILD SUCCESSFUL!
