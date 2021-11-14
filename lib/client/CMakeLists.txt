set(TargetName SphereDefender)
find_package(SFML COMPONENTS graphics PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/cursor_button.cpp
    src/event_handler.cpp
    src/game_manager.cpp
    src/game.cpp
    src/gui.cpp
    src/lobby.cpp
    src/main_menu.cpp
    src/main.cpp
    src/settings.cpp
    src/spritesheet.cpp
    src/textbox.cpp
    src/util.cpp
    src/world_map.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/client/include
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
)

target_link_libraries(${TargetName}
    network
    sfml-graphics
)
