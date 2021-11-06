set(TargetName SphereDefender)
find_package(SFML COMPONENTS graphics PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/cursor_button.cpp
    src/event_handler.cpp
    src/game_manager.cpp
    src/lobby.cpp
    src/main_menu.cpp
    src/main.cpp
    src/spritesheet.cpp
    src/textbox.cpp
    src/util.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    include
    externals/sfml/install/include
)

target_link_libraries(${TargetName} sfml-graphics)
