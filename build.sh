#!/bin/sh
# =============================================================================
#  COMMON-C UNIX BUILD SCRIPT
#  ELASTIC SOFTWORKS 2025
#
#  This script provides a simple way to compile the COMMON-C library
#  on Unix-like systems (Linux, macOS, etc.). It assumes that a `make`
#  utility is available in the system's PATH.
#
#  Usage:
#    ./build.sh          - Compiles the library in release mode.
#    ./build.sh debug    - Compiles the library in debug mode.
#    ./build.sh clean    - Removes all build artifacts.
#    ./build.sh install  - (If defined in Makefile) Installs the library.
# =============================================================================

echo "--- COMMON-C UNIX BUILD ---"

# check if a specific target was passed (e.g., "debug", "clean")

if [ -z "$1" ]; then
    echo "BUILDING IN RELEASE MODE..."
    make
else
    echo "BUILDING TARGET: $1..."
    make "$1"
fi

# check the exit code of the make command

if [ $? -ne 0 ]; then
    echo ""
    echo "BUILD FAILED!"
    exit 1
fi

echo ""
echo "BUILD SUCCESSFUL!"
