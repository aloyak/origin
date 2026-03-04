#!/bin/sh
echo "Building Project..."

mkdir build && cd build
cmake ..
cmake --build .

echo "Build Complete!"