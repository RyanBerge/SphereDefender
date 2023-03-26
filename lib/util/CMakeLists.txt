set(TargetName util)
find_package(SFML COMPONENTS graphics PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/game_math.cpp
    src/pathfinding.cpp
)

add_library(${TargetName} SHARED ${Sources})

target_include_directories(${TargetName} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/util/include
)

target_link_libraries(${TargetName} sfml-graphics)
