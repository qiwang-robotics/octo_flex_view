# OctoFlexView

**现代化、易用的Qt5 OpenGL 3D可视化库**

[简体中文](README_zh.md) | [English](README.md)

[![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/cpp/17)
[![Qt](https://img.shields.io/badge/Qt-5-green)](https://www.qt.io/)

OctoFlexView是一个基于Qt5和OpenGL固定管线的模块化3D可视化库，提供简洁的API和强大的功能。

## 特性

- **零配置启动** - 1行代码创建3D查看器
- **链式API** - 流畅的Builder模式对象构建
- **简化纹理加载** - 自动处理OpenGL上下文
- **CMake集成** - 3行配置完成项目集成
- **多视图支持** - 动态分割/合并视图
- **丰富的几何体** - 球体、立方体、圆柱、圆锥等
- **线程安全** - ObjectManager提供线程安全访问

---

## 快速开始

### 最小示例（11行代码）

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

### Builder模式示例

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

### 纹理加载示例

```cpp
auto textured = ObjectBuilder::begin("textured")
    .texturedQuad(3.0, 3.0, "assets/texture.png", 0.8)
    .at(-2, 0, 0)
    .build();

viewer.add(textured);
```

---

## 构建和安装

### 依赖项

- **Qt5** (Widgets, OpenGL)
- **OpenGL**
- **GLEW** (OpenGL Extension Wrangler)
- **GLM** (OpenGL Mathematics)
- **CMake** 3.10+
- **C++17** 编译器

### Qt 许可说明

本项目依赖 Qt 的开源许可（通常为 LGPL）。如果你在 LGPL 下发布包含 Qt 的二进制，
建议采用动态链接并确保用户可以替换 Qt 库。

### Ubuntu/Debian

```bash
sudo apt-get install qtbase5-dev qt5-qmake libqt5opengl5-dev \
                     libgl1-mesa-dev libglu1-mesa-dev libglew-dev \
                     cmake make g++
```

### 编译

```bash
git clone https://github.com/qiwang-robotics/octo_flex_view.git
cd octo_flex_view
mkdir build && cd build
cmake ..
make -j4
```

### 安装（可选）

```bash
sudo make install
```

安装后，您的项目CMakeLists.txt只需3行：

```cmake
find_package(OctoFlexView REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app OctoFlexView::octo_flex_view)
```

### 卸载

如需卸载已安装的库：

```bash
sudo make uninstall
```

卸载目标会：
- 删除所有已安装的文件（库、头文件、CMake配置）
- 自动清理空目录
- 提供详细的卸载反馈信息

---

## 项目结构

```
OctoFlexView/
├── include/              # 公共API头文件
├── src/                  # 实现文件
├── example/              # 示例程序
│   ├── simple_viewer/
│   ├── embedded_viewer/
│   └── dynamic_bar_chart/
├── cmake/                # CMake配置
└── CMakeLists.txt        # 主构建脚本
```

---

## 示例程序

### simple_viewer - 最小独立查看器（73行）

自动管理窗口和事件循环，展示基础几何体和纹理。

```bash
cd example/simple_viewer
mkdir -p build && cd build
cmake .. && make
./simple_viewer
```

### embedded_viewer - 嵌入自定义Qt窗口

展示如何将查看器集成到您自己的Qt应用程序中。

```bash
cd example/embedded_viewer
mkdir -p build && cd build
cmake .. && make
./embedded_viewer
```

功能：
- 左侧按钮面板支持动态对象操作
- 清晰的模块化架构
- 演示 `createEmbedded()` API 的集成用法

### dynamic_bar_chart - 动态3D可视化

显示带波浪动画的柱状图网格。

```bash
cd example/dynamic_bar_chart
mkdir -p build && cd build
cmake .. && make
./dynamic_bar_chart
```

功能：
- 8x12 柱状图网格（共96根柱子）
- 根据高度颜色映射（蓝→绿→橙）
- 约30 FPS的流畅波浪动画
- 演示实时对象更新

---

## 架构设计

### API层次

1. **QuickStart API** (`OctoFlexViewer`) - 零配置，适合快速原型
2. **Fluent Builder API** (`ObjectBuilder`) - 链式调用，适合复杂场景

### 核心组件

- **OctoFlexViewContainer**: 视图容器，支持分割/合并
- **ObjectManager**: 对象管理器，线程安全
- **Layer**: 图层系统，控制可见性和可选择性
- **Object**: 3D对象，包含多个Shape
- **Shape**: 几何形状，支持纹理

### 设计模式

- **Facade模式**: `OctoFlexViewer` 简化初始化
- **Builder模式**: `ObjectBuilder` 流畅构建
- **Repository模式**: `ObjectManager` 集中存储
- **Composite模式**: `OctoFlexViewContainer` 视图管理
- **Observer模式**: 对象选择和图层可见性

---

## 高级用法

### 访问底层组件

```cpp
auto viewer = OctoFlexViewer::create("Advanced");

// 访问ObjectManager（线程安全操作）
auto objManager = viewer.objectManager();
objManager->submit(object, "layer_id");

// 容器控件（用于嵌入到其他UI中）
QWidget* container = viewer.container();
// 可以添加到其他Qt布局中：
// myLayout->addWidget(container);
```

### 多图层管理

```cpp
viewer.add(foreground_obj, "foreground");
viewer.add(background_obj, "background");
viewer.add(ui_obj, "ui");

// 图层可见性控制可通过ObjectManager进行
auto objManager = viewer.objectManager();
// 可以通过对象管理器接口管理图层
```

---

## 视频录制

OctoFlexView 支持容器级视频录制，正确渲染透明物体。

### 基础录制

```cpp
auto viewer = OctoFlexViewer::create("录制演示");

// 添加透明物体
viewer.addSphere("transparent_sphere", Vec3(0, 0, 0), 1.0, 0.5);  // 50% 透明

// 开始录制
octo_flex::RecordingOptions options;
options.output_path = "output.mp4";
options.fps = 30;
viewer.startRecording(options);

// ... 你的动画或交互 ...

// 停止录制
viewer.stopRecording();
```

### 录制选项

```cpp
octo_flex::RecordingOptions options;
options.output_path = "output.mp4";  // 输出文件路径
options.fps = 30;                     // 帧率
options.codec = "libx264";            // 视频编码器
options.preset = "veryfast";          // 编码预设
options.crf = 23;                     // 质量（越低越好，推荐 18-28）
options.overwrite = true;             // 覆盖已存在文件

viewer.startRecording(options);
```

### 暂停和恢复

```cpp
viewer.pauseRecording();   // 暂停录制
viewer.resumeRecording();  // 恢复录制
```

### 录制状态

```cpp
bool isRecording = viewer.isRecording();      // 是否正在录制
bool isPaused = viewer.isRecordingPaused();   // 是否已暂停
std::string error = viewer.getLastRecordingError();  // 获取错误信息
```

### 技术说明

- 使用 `glReadPixels` 直接从 OpenGL 帧缓冲区捕获帧
- 透明物体通过正确的深度排序渲染
- 两遍渲染：先渲染不透明物体，再渲染透明物体
- 需要安装 `ffmpeg` 并在 PATH 中可用

---

## 许可证

本项目采用 Apache License 2.0 - 详见 [LICENSE](LICENSE) 文件。

---

## 贡献

本项目正在积极开发中。
暂不接受外部Pull Request。

欢迎通过Issues提出反馈和建议。

---

## 联系方式

如有疑问，请提交Issue。

- **项目主页**: https://github.com/qiwang-robotics/octo_flex_view
- **问题反馈**: https://github.com/qiwang-robotics/octo_flex_view/issues

---
