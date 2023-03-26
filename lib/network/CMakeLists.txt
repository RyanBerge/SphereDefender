set(TargetName network)
find_package(SFML COMPONENTS network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/messaging.cpp
)

add_library(${TargetName} SHARED ${Sources})

target_include_directories(${TargetName} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/network/include
)

target_link_libraries(${TargetName}
    util
    definitions
    sfml-network
)
