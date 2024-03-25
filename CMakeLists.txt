cmake_minimum_required(VERSION 3.0.0)

project(simulate2 LANGUAGES C CXX)

# Default to GLVND if available.
cmake_policy(SET CMP0072 NEW)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add glad
add_library(glad external/glad/src/glad.c)

# Add glfw3
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)

# Add imgui
set(IMGUI_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
)

# Add fmt
add_subdirectory(external/fmt)

set(IMGUI_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_demo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp
        Gui.h
        Gui.cpp
)

find_package(mujoco REQUIRED)

include_directories(
        external
        external/glad/include
        external/fmt/include
        ${GLFW_INCLUDE_DIRS}
        ${IMGUI_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
        main.cpp
        ${IMGUI_SRCS}
        Input.cpp
        ViewportRoutines.cpp
)

target_link_libraries(${PROJECT_NAME}
        glad
        glfw
        mujoco::mujoco
        fmt
)

# Copy resources on change
add_custom_target(copy-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
)
add_dependencies(${PROJECT_NAME} copy-files)
