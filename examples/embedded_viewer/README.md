# Embedded Viewer Example

A minimal example showing how to embed OctoFlexView into an existing Qt application.

## Key Concept

Use `OctoFlexViewer::createEmbedded()` when you want to integrate the 3D viewer into your own Qt window, instead of having it create its own window.

## Build & Run

```bash
cd example/embedded_viewer
mkdir build && cd build
cmake ..
make
./embedded_viewer
```

## Code Overview

```cpp
// 1. You manage QApplication
QApplication app(argc, argv);

// 2. You create the window
QMainWindow window;

// 3. Create embedded viewer (no auto-window)
auto viewer = OctoFlexViewer::createEmbedded(container);

// 4. Add objects
viewer.addSphere("sphere", Vec3(1, 0, 0), 1.0);

// 5. Embed the widget
layout->addWidget(viewer.widget());

// 6. You run the event loop
return app.exec();
```

## Comparison

| Mode | Who creates window? | Who runs event loop? |
|------|---------------------|----------------------|
| Standalone | Library (`create()`) | Library (`viewer.run()`) |
| Embedded | You (`createEmbedded()`) | You (`app.exec()`) |
