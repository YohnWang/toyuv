#bash

if [ ! -e build ];then
    cmake -S . -B build -G "MinGW Makefiles"
fi
cmake --build build -t run