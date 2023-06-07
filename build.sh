#! /bin/bash
mkdir build
cmake -B ./build -S . -DCMAKE_TOOLCHAIN_FILE=/home/group01/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build ./build/
exit 0