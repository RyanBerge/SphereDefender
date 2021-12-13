set(TargetName network)
find_package(SFML COMPONENTS network PATHS ${PROJECT_SOURCE_DIR}/externals/sfml/install)

set(Sources
    src/messaging.cpp
)

add_library(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/network/include
    ${PROJECT_SOURCE_DIR}/lib/shared/include
)

target_link_libraries(${TargetName} sfml-network)
