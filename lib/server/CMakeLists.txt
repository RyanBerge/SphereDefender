set(TargetName Server)
find_package(SFML COMPONENTS network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/main.cpp
    src/server.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/server/include
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
)

target_link_libraries(${TargetName}
    network
    sfml-network
)
