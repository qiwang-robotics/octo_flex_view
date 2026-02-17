cmake_minimum_required(VERSION 3.10.0)
project(octo_flex_view VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set OpenGL policy to prefer GLVND
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)
find_package(glm REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

# Enable automatic Qt MOC processing.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create shared library
add_library(octo_flex_view SHARED
    src/def.cpp
    src/utils.cpp
    src/shape.cpp
    src/textured_quad.cpp
    src/object.cpp
    src/layer.cpp
    src/camera.cpp
    src/coordinate_system.cpp
    src/object_manager.cpp
    src/info_panel.cpp
    src/octo_flex_view.cpp
    src/octo_flex_view_container.cpp
    src/object_tree_dialog.cpp
    src/octo_flex_viewer.cpp
    src/object_builder.cpp
    src/builtin_textures.cpp
    src/video_recorder.cpp
    src/recording_thread.cpp
)

# Link libraries
target_link_libraries(octo_flex_view
    PUBLIC ${OPENGL_LIBRARIES}
    PUBLIC GLEW::GLEW
    PUBLIC Qt5::Widgets
    PUBLIC Qt5::OpenGL
    PUBLIC pthread
)

# ============================================================================
# Installation Configuration
# ============================================================================

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Install library
install(TARGETS octo_flex_view
    EXPORT OctoFlexViewTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/octo_flex
    FILES_MATCHING PATTERN "*.h"
)

# Generate and install CMake config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OctoFlexViewConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OctoFlexViewConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OctoFlexView
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/OctoFlexViewConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OctoFlexViewConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OctoFlexViewConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OctoFlexView
)

install(EXPORT OctoFlexViewTargets
    FILE OctoFlexViewTargets.cmake
    NAMESPACE OctoFlexView::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OctoFlexView
)

# ============================================================================
# Uninstall target
# ============================================================================

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    COMMENT "Uninstalling OctoFlexView..."
)
