set(TargetName Server)
find_package(SFML COMPONENTS graphics network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    main.cpp
    server.cpp
    ../global_resources.cpp
    ../network.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/Server
)

target_link_libraries(${TargetName} sfml-graphics sfml-network)
