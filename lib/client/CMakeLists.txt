set(TargetName SphereDefender)
find_package(SFML COMPONENTS graphics PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(Sources
    src/avatar.cpp
    src/cursor_button.cpp
    src/enemy.cpp
    src/event_handler.cpp
    src/game_manager.cpp
    src/game.cpp
    src/gui.cpp
    src/lobby.cpp
    src/main_menu.cpp
    src/main.cpp
    src/player.cpp
    src/resources.cpp
    src/settings.cpp
    src/spritesheet.cpp
    src/textbox.cpp
    src/region_map.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/client/include
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/lib/shared/include
    ${PROJECT_SOURCE_DIR}/lib/util/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
)

target_link_libraries(${TargetName}
    network
    shared
    sfml-graphics
    Threads::Threads
)
