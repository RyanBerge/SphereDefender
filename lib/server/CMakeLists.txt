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

target_include_directories(${TargetName} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/server/include
)

target_link_libraries(${TargetName}
    network
    util
    definitions
    sfml-network
)
