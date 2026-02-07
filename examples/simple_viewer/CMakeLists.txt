cmake_minimum_required(VERSION 3.10)
project(OctoFlexViewExamples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set OpenGL policy to prefer GLVND
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)

# Find OpenGL
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories (only public API headers)
include_directories(
    ${CMAKE_SOURCE_DIR}/../../include
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

# Link directories
link_directories(${CMAKE_SOURCE_DIR}/../../build)

# Add resources (needed for texture loading)
#set(RESOURCES ../resources.qrc)

# Simple viewer example
add_executable(simple_viewer simple_viewer.cpp ${RESOURCES})
target_link_libraries(simple_viewer
    octo_flex_view
    Qt5::Widgets
    Qt5::OpenGL
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    pthread
)
