cmake_minimum_required(VERSION 3.10)
project(dynamic_bar_chart)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set OpenGL policy to prefer GLVND
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Find required packages
find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Include directories (only public API headers)
include_directories(
    ${CMAKE_SOURCE_DIR}/../../include
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

# Link directories (point to build directory)
link_directories(${CMAKE_SOURCE_DIR}/../../build)

# Create executable
add_executable(dynamic_bar_chart main.cpp)

# Link libraries
target_link_libraries(dynamic_bar_chart
    octo_flex_view
    Qt5::Widgets
    Qt5::OpenGL
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    pthread
)
