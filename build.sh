#!/bin/bash
DIR=./build

if [ -d "$DIR" ];
then
    cd $DIR
else
	mkdir $DIR && cd $DIR
fi

# установка пакетов conan
conan install .. -s compiler.libcxx=libstdc++11 -s build_type=Debug

# сборка
cmake .. -DCMAKE_BUILD_TYPE=Debug

# cmake --build . -t game_server
# cmake --build . -t tests
cmake --build .