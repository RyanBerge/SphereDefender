#!/bin/bash

COMMAND="build"

for i in "$@"
do
    case $i in
        "--rebuild"*)
        COMMAND="rebuild"
        shift
        ;;
        "--clean"*)
        COMMAND="clean"
        shift
        ;;
        *)
        echo "Unknown option: ${i}"
        shift
        ;;
    esac
done

clean() {
    rm -rf build
    rm -rf externals/sfml/install
    rm -rf externals/sfml/SFML/cmake/build
}

build() {
    # Put the mingw32 toolchain in our PATH so we can use it
    export PATH="/mnt/c/Program Files (x86)/mingw-w64/i686-8.1.0-posix-dwarf-rt_v6-rev0/mingw32/bin:${PATH}"

    if [ ! -d externals/sfml/install ]; then
        mkdir -p externals/sfml/install
        mkdir -p externals/sfml/SFML/cmake/build

        pushd externals/sfml/SFML/cmake/build

        cmake.exe \
            -G "MinGW Makefiles" \
            -DCMAKE_INSTALL_PREFIX=../../../install \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            ../..

        mingw32-make.exe
        mingw32-make.exe install

        popd
    fi

    mkdir -p build
    pushd build
    cmake.exe -G "MinGW Makefiles" ../
    mingw32-make.exe
    popd
    cp externals/sfml/install/bin/*.dll build/bin/
}

rebuild() {
    rm -rf build
    build
}

eval "${COMMAND}"
