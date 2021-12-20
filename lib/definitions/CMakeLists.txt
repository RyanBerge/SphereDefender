set(TargetName definitions)
find_package(nlohmann_json 3.2.0 REQUIRED PATHS ${PROJECT_SOURCE_DIR}/externals/Json/install)

set(Sources
    src/entity_definitions.cpp
    src/region_definitions.cpp
)

add_library(${TargetName} ${Sources})

target_include_directories(${TargetName} PRIVATE
    ${PROJECT_SOURCE_DIR}/lib/definitions/include
    ${PROJECT_SOURCE_DIR}/externals/sfml/install/include
    ${PROJECT_SOURCE_DIR}/externals/Json/install/include
)

target_link_libraries(${TargetName}
    nlohmann_json::nlohmann_json
)
