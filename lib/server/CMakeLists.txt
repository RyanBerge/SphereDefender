set(TargetName Server)
find_package(SFML COMPONENTS network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/new_enemy.cpp
    src/main.cpp
    src/global_state.cpp
    src/player.cpp
    src/region.cpp
    src/server.cpp
    src/util.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/server/include
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/lib/util/include
    ${PROJECT_SOURCE_DIR}/lib/definitions/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
)

target_link_libraries(${TargetName}
    network
    util
    definitions
    sfml-network
)
