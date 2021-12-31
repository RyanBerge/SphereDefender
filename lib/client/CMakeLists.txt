set(TargetName SphereDefender)
find_package(SFML COMPONENTS graphics PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)
find_package(nlohmann_json 3.2.0 REQUIRED PATHS ${PROJECT_SOURCE_DIR}/externals/Json/install)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(Sources
    src/avatar.cpp
    src/convoy.cpp
    src/cursor_button.cpp
    src/enemy.cpp
    src/event_handler.cpp
    src/game_manager.cpp
    src/game.cpp
    src/gui.cpp
    src/lobby.cpp
    src/main_menu.cpp
    src/main.cpp
    src/overmap.cpp
    src/player.cpp
    src/resources.cpp
    src/settings.cpp
    src/spritesheet.cpp
    src/stash.cpp
    src/textbox.cpp
    src/region_map.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/client/include
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/lib/definitions/include
    ${PROJECT_SOURCE_DIR}/lib/util/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
    ${PROJECT_SOURCE_DIR}/externals/Json/install/include
)

target_link_libraries(${TargetName}
    network
    definitions
    util
    sfml-graphics
    Threads::Threads
    nlohmann_json::nlohmann_json
)
