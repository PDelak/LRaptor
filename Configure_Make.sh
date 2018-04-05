#!/bin/sh

echo "Configuring CMake for make."
mkdir build-make
cd build-make
cmake -G "Unix Makefiles" ..
