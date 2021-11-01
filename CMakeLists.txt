cmake_minimum_required(VERSION 3.12)
project (SphereDefender VERSION 1.0 LANGUAGES CXX)
add_compile_options(-Wall -Wextra -pedantic)
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

add_subdirectory(src/Server)
add_subdirectory(src/Client)
