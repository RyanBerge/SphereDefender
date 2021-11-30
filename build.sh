#!/bin/bash

COMMAND="build"
BUILD=Debug

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
        "--release"*)
        BUILD=Release
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
    rm -rf bin
    rm -rf publish
    rm -rf externals/sfml/install
    rm -rf externals/sfml/SFML/cmake/build
}

build() {
    if [ ! -f externals/sfml/SFML/CMakeLists.txt ]; then
        echo "Please initialize all submodules."
        exit
    fi

    if [ ! -d externals/sfml/install ]; then
        mkdir -p externals/sfml/install
        mkdir -p externals/sfml/SFML/cmake/build

        pushd externals/sfml/SFML/cmake/build

        cmake.exe \
            -G "MinGW Makefiles" \
            -DCMAKE_INSTALL_PREFIX=../../../install \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DCMAKE_BUILD_TYPE=${BUILD} \
            ../..

        mingw32-make.exe --no-print-directory
        mingw32-make.exe install --no-print-directory

        popd
    fi

    mkdir -p build
    pushd build
    cmake.exe -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=${BUILD} ../
    mingw32-make.exe --no-print-directory
    popd
    cp externals/sfml/install/bin/*.dll bin/

    if [ ${BUILD} == Release ]; then
        echo "Publishing..."
        rm -rf publish
        mkdir -p publish/bin
        mkdir -p publish/assets
        cp -r assets/*.png publish/assets
        cp -r assets/*.ttf publish/assets
        cp bin/*.exe publish/bin/
        cp bin/*.dll publish/bin/
        cp /mnt/c/WinLibs/mingw32/bin/libgcc_s_dw2-1.dll publish/bin
        cp /mnt/c/WinLibs/mingw32/bin/libstdc++-6.dll publish/bin
        cp /mnt/c/WinLibs/mingw32/bin/libwinpthread-1.dll publish/bin

        dig +short myip.opendns.com @resolver1.opendns.com > publish/bin/config.ini
    fi
}

rebuild() {
    rm -rf build
    rm -rf bin
    rm -rf publish
    build
}

eval "${COMMAND}"
