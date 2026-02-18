cmake_minimum_required(VERSION 3.25)
project(Engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for Clangd / Cursor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/FetchDependencies.cmake)

add_executable(engine)
target_compile_options(engine PRIVATE -Wall -Wextra -Werror)

add_subdirectory(src)

target_link_libraries(engine PRIVATE glfw glad glm imgui stb assimp)

# Absolute path to the assets directory so shaders can be found regardless of
# the working directory from which the executable is invoked.
target_compile_definitions(engine PRIVATE
    ENGINE_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS)

# Copy compile_commands.json to project root for Clangd
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS engine)
