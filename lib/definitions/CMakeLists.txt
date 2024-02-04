set(TargetName definitions)
find_package(nlohmann_json 3.2.0 REQUIRED PATHS ${PROJECT_SOURCE_DIR}/externals/Json/install)

set(Sources
    src/animation_tracker.cpp
    src/debug_overrides.cpp
    src/definitions.cpp
)

add_library(${TargetName} SHARED ${Sources})

target_include_directories(${TargetName} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/definitions/include
)

target_link_libraries(${TargetName}
    nlohmann_json::nlohmann_json
    util
)
