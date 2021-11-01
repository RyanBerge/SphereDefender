set(TargetName Client)
find_package(SFML COMPONENTS graphics network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    cursor_button.cpp
    game_manager.cpp
    game.cpp
    lobby.cpp
    main_menu.cpp
    main.cpp
    messaging.cpp
    player_states.cpp
    player.cpp
    spritesheet.cpp
    state_manager.cpp
    textbox.cpp
    ../global_resources.cpp
    ../network.cpp
)

add_executable(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/Client
)

target_link_libraries(${TargetName} sfml-graphics sfml-network)
