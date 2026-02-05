add_library(base)
target_sources(base
    PUBLIC
    FILE_SET CXX_MODULES
    FILES base.cpp
)
set_target_properties(base PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_library(algo)
target_sources(algo
    PUBLIC
    FILE_SET CXX_MODULES
    FILES algo.cpp
)
target_link_libraries(algo PRIVATE base)
set_target_properties(algo PROPERTIES CXX_SCAN_FOR_MODULES ON)


add_library(anchor_core)
target_sources(anchor_core
    PUBLIC
    FILE_SET CXX_MODULES
    FILES core.cpp
)
target_link_libraries(anchor_core PRIVATE base algo)
set_target_properties(anchor_core PROPERTIES CXX_SCAN_FOR_MODULES ON)

add_library(renderer)
target_sources(renderer
    PUBLIC
    FILE_SET CXX_MODULES
    FILES renderer.cpp 
)
target_link_libraries(renderer PRIVATE app base anchor_core)
set_target_properties(renderer PROPERTIES CXX_SCAN_FOR_MODULES ON)


add_library(app)
target_sources(app
    PUBLIC
    FILE_SET CXX_MODULES
    FILES app.cpp
)
target_link_libraries(app PRIVATE base anchor_core)
set_target_properties(app PROPERTIES CXX_SCAN_FOR_MODULES ON)

# target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE app anchor_core renderer raylib)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_SCAN_FOR_MODULES ON)
