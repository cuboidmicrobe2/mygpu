#!/bin/sh

set -e

make

cmake -S renderer -B build/renderer-cmake
cmake --build build/renderer-cmake
ctest --test-dir build/renderer-cmake --output-on-failure