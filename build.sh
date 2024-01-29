#bash

if [[ "$1" == "--clean" ]];then
    rm -rf build
fi

if [ ! -e build ];then
    os=$(uname)
    if [[ "$os" == *MINGW* ]];then
        cmake -S . -B build -G "MinGW Makefiles"
    else
        cmake -S . -B build -G Ninja
    fi
fi
cmake --build build -t run