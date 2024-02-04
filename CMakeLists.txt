cmake_minimum_required(VERSION 3.12)
project (SphereDefender VERSION 1.0 LANGUAGES CXX)
add_compile_options(-Wall -Wextra -Wshadow -pedantic)
add_compile_options (-fdiagnostics-color=always)
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

add_subdirectory(lib)
