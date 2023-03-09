#!/bin/bash

COMMAND="build"
BUILD=Debug
ARCHITECTURE=Win32

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
        "--linux"*)
        ARCHITECTURE=Linux
        shift
        ;;
        *)
        echo "Unknown option: ${i}"
        shift
        ;;
    esac
done

clean() {
    sudo rm -rf build
    sudo rm -rf bin
    sudo rm -rf publish
    sudo rm -rf externals/sfml/install
    sudo rm -rf externals/sfml/SFML/cmake/build
    sudo rm -rf externals/Json/install
    sudo rm -rf externals/Json/json/cmake/build
}

build() {
    if [ ! -f externals/sfml/SFML/CMakeLists.txt ]; then
        echo "Please initialize all submodules."
        exit
    fi

    GENERATOR="MinGW Makefiles"
    MAKE=mingw32-make.exe
    CMAKE=cmake.exe

    if [ ${ARCHITECTURE} == Linux ]; then
        GENERATOR="Unix Makefiles"
        MAKE=make
        CMAKE=cmake
    fi

    if [ ! -d externals/sfml/install ]; then
        mkdir -p externals/sfml/install
        mkdir -p externals/sfml/SFML/cmake/build

        pushd externals/sfml/SFML/cmake/build

        ${CMAKE} \
            -G "${GENERATOR}" \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_INSTALL_PREFIX=../../../install \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DCMAKE_BUILD_TYPE=${BUILD} \
            ../..

        ${MAKE} --no-print-directory
        ${MAKE} install --no-print-directory

        popd
    fi

    if [ ! -d externals/Json/install ]; then
        mkdir -p externals/Json/install
        mkdir -p externals/Json/json/cmake/build

        pushd externals/Json/json/cmake/build

        ${CMAKE} \
            -G "${GENERATOR}" \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_INSTALL_PREFIX=../../../install \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DCMAKE_BUILD_TYPE=${BUILD} \
            -DJSON_BuildTests=OFF \
            ../..

        ${MAKE} --no-print-directory
        ${MAKE} install --no-print-directory

        popd
    fi

    mkdir -p build
    pushd build
    ${CMAKE} -G "${GENERATOR}" -DCMAKE_BUILD_TYPE=${BUILD} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
    ${MAKE} --no-print-directory
    popd

    if [ ${ARCHITECTURE} == Win32 ]; then
        cp externals/sfml/install/bin/*.dll bin/
        cp /mnt/c/WinLibs/mingw32/bin/libgcc_s_dw2-1.dll bin/
        cp /mnt/c/WinLibs/mingw32/bin/libstdc++-6.dll bin/
        cp /mnt/c/WinLibs/mingw32/bin/libwinpthread-1.dll bin/
    elif [ ${ARCHITECTURE} == Linux ]; then
        cp externals/sfml/install/lib/*.so* bin/
    fi

    if [ ${BUILD} == Release ]; then
        echo "Publishing..."
        rm -rf publish
        mkdir -p publish/bin
        mkdir -p publish/assets
        mkdir -p publish/data
        cp -r assets/*.png publish/assets
        cp -r assets/*.ttf publish/assets
        cp -r data/* publish/data

        if [ ${ARCHITECTURE} == Win32 ]; then
            cp bin/*.exe publish/bin/
            cp bin/*.dll publish/bin/
        elif [ ${ARCHITECTURE} == Linux ]; then
            cp bin/* publish/bin/
        fi

        cp /mnt/c/WinLibs/mingw32/bin/libgcc_s_dw2-1.dll publish/bin
        cp /mnt/c/WinLibs/mingw32/bin/libstdc++-6.dll publish/bin
        cp /mnt/c/WinLibs/mingw32/bin/libwinpthread-1.dll publish/bin

        dig +short myip.opendns.com @resolver1.opendns.com > publish/bin/config.ini
    fi
}

rebuild() {
    sudo rm -rf build
    sudo rm -rf bin
    sudo rm -rf publish
    build
}

eval "${COMMAND}"
