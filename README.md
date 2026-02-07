# OctoFlexView

**Modern, easy-to-use Qt5 OpenGL 3D visualization library**

[简体中文](README_zh.md) | [English](README.md)

[![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/cpp/17)
[![Qt](https://img.shields.io/badge/Qt-5-green)](https://www.qt.io/)

OctoFlexView is a modular 3D visualization library built on Qt5 and the fixed-function OpenGL pipeline. It provides a clean API with powerful features.

## Features

- **Zero-config startup** - Create a 3D viewer in one line
- **Fluent API** - Smooth builder-style object construction
- **Simplified texture loading** - Automatic OpenGL context handling
- **CMake integration** - Integrate in 3 lines of CMake
- **Multi-view support** - Dynamic split/merge views
- **Rich primitives** - Sphere, box, cylinder, cone, and more
- **Thread safety** - ObjectManager provides thread-safe access

---

## Quick Start

### Minimal example (11 lines)

```cpp
#include "octo_flex_viewer.h"
using namespace octo_flex;

int main(int argc, char* argv[]) {
    auto viewer = OctoFlexViewer::create("Hello OctoFlexView");
    viewer.addSphere("sphere", Vec3(1, 0, 0), 1.0);
    viewer.show();
    return viewer.run();
}
```

### Builder pattern example

```cpp
auto viewer = OctoFlexViewer::create("Builder Demo", 1280, 720);

auto obj = ObjectBuilder::begin("complex_object")
    .sphere(Vec3(1, 0, 0), 1.0)
    .at(2, 0, 0)
    .rotateZ(M_PI / 4)
    .withInfo("Rotated Sphere")
    .build();

viewer.add(obj).show();
return viewer.run();
```

### Texture loading example

```cpp
auto textured = ObjectBuilder::begin("textured")
    .texturedQuad(3.0, 3.0, "assets/texture.png", 0.8)
    .at(-2, 0, 0)
    .build();

viewer.add(textured);
```

---

## Build and Install

### Dependencies

- **Qt5** (Widgets, OpenGL)
- **OpenGL**
- **GLEW** (OpenGL Extension Wrangler)
- **GLM** (OpenGL Mathematics)
- **CMake** 3.10+
- **C++17** compiler

### Qt licensing note

This project depends on Qt under its open-source license (typically LGPL). If you
distribute binaries using Qt under LGPL, prefer dynamic linking and ensure users
can replace the Qt libraries.

### Ubuntu/Debian

```bash
sudo apt-get install qtbase5-dev qt5-qmake libqt5opengl5-dev \
                     libgl1-mesa-dev libglu1-mesa-dev libglew-dev \
                     cmake make g++
```

### Build

```bash
git clone https://github.com/qiwang-robotics/octo_flex_view.git
cd octo_flex_view
mkdir build && cd build
cmake ..
make -j4
```

### Install (optional)

```bash
sudo make install
```

After installation, add just three lines to your project's CMakeLists.txt:

```cmake
find_package(OctoFlexView REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app OctoFlexView::octo_flex_view)
```

### Uninstall

To remove the installed library:

```bash
sudo make uninstall
```

The uninstall target will:
- Remove all installed files (library, headers, CMake config)
- Automatically clean empty directories
- Provide detailed uninstall feedback

---

## Project Structure

```
OctoFlexView/
├── include/              # Public API headers
├── src/                  # Implementation
├── example/              # Example apps
│   ├── simple_viewer/
│   ├── embedded_viewer/
│   └── dynamic_bar_chart/
├── cmake/                # CMake config
└── CMakeLists.txt        # Main build script
```

---

## Examples

### simple_viewer - minimal standalone viewer (73 lines)

Auto-managed window and event loop with basic geometry and textures.

```bash
cd example/simple_viewer
mkdir -p build && cd build
cmake .. && make
./simple_viewer
```

### embedded_viewer - embed in custom Qt window

Shows how to integrate the viewer into your own Qt application with custom UI.

```bash
cd example/embedded_viewer
mkdir -p build && cd build
cmake .. && make
./embedded_viewer
```

Features:
- Left-side button panel for dynamic object manipulation
- Clean modular architecture with separated concerns
- Usage of `createEmbedded()` API for integration

### dynamic_bar_chart - animated 3D visualization

Displays a grid of animated bars with wave motion.

```bash
cd example/dynamic_bar_chart
mkdir -p build && cd build
cmake .. && make
./dynamic_bar_chart
```

Features:
- 8x12 grid of bars (96 total)
- Color-mapped by height (blue → green → orange)
- Smooth wave animation at ~30 FPS
- Demonstrates real-time object updates

---

## Architecture

### API Layers

1. **QuickStart API** (`OctoFlexViewer`) - zero-config, fast prototyping
2. **Fluent Builder API** (`ObjectBuilder`) - fluent construction for complex scenes

### Core Components

- **OctoFlexViewContainer**: View container, supports split/merge
- **ObjectManager**: Object manager with thread safety
- **Layer**: Layer system controlling visibility/selectability
- **Object**: 3D object composed of multiple shapes
- **Shape**: Geometry shape with optional textures

### Design Patterns

- **Facade**: `OctoFlexViewer` simplifies initialization
- **Builder**: `ObjectBuilder` provides fluent construction
- **Repository**: `ObjectManager` centralizes storage
- **Composite**: `OctoFlexViewContainer` manages view layouts
- **Observer**: Object selection and layer visibility

---

## Advanced Usage

### Access low-level components

```cpp
auto viewer = OctoFlexViewer::create("Advanced");

// ObjectManager access (thread-safe)
auto objManager = viewer.objectManager();
objManager->submit(object, "layer_id");

// Container widget (for embedding in other UIs)
QWidget* container = viewer.container();
// Can be added to other Qt layouts:
// myLayout->addWidget(container);
```

### Multi-layer management

```cpp
viewer.add(foreground_obj, "foreground");
viewer.add(background_obj, "background");
viewer.add(ui_obj, "ui");

// Layer visibility control available through ObjectManager
auto objManager = viewer.objectManager();
// Layers can be managed through the object manager interface
```

---

## License

This project is licensed under the Apache License 2.0 - see [LICENSE](LICENSE) for details.

---

## Contributing

This project is currently under active development.
External pull requests are not accepted at this stage.

Feedback and suggestions are welcome via Issues.

---

## Contact

For serious inquiries, please open an issue.

- **Project home**: https://github.com/qiwang-robotics/octo_flex_view
- **Issues**: https://github.com/qiwang-robotics/octo_flex_view/issues

---
