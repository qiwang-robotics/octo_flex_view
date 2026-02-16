// SPDX-License-Identifier: Apache-2.0
//
// Copyright (c) 2026 Qi Wang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OCTO_FLEX_VIEWER_H
#define OCTO_FLEX_VIEWER_H

#include <functional>
#include <memory>
#include <string>

#include "octo_flex_export.h"
#include "def.h"
#include "recording_options.h"

class QWidget;

namespace octo_flex {

// Forward declarations
class ObjectManager;
class Object;
class OctoFlexViewContainer;

/**
 * @brief Embedded viewer for integration into existing Qt applications
 *
 * Provides a QWidget-based viewer that can be embedded into user's own Qt windows.
 * Does not create its own window or manage the event loop - full control stays with the user.
 * Hides internal implementation details like ObjectManager for a clean API.
 *
 * @example Embedded usage:
 * @code
 * // User manages QApplication and windows
 * QApplication app(argc, argv);
 * QMainWindow* myWindow = new QMainWindow();
 *
 * // Create embedded viewer
 * auto embeddedViewer = OctoFlexViewer::createEmbedded(myWindow);
 *
 * // Add objects using fluent API
 * embeddedViewer.addSphere("sphere1", Vec3(1, 0, 0), 1.0)
 *               .addBox("box1", Vec3(0, 1, 0), 1, 1, 1);
 *
 * // Embed into user's window
 * myWindow->setCentralWidget(embeddedViewer.widget());
 *
 * // User controls window and event loop
 * myWindow->show();
 * return app.exec();
 * @endcode
 */
class OCTO_FLEX_VIEW_API EmbeddedViewer {
   public:
    /**
     * @brief Add object to the default layer (fluent API)
     *
     * @param object Object to add
     * @return Reference to this viewer (for chaining)
     *
     * @example
     * @code
     * embeddedViewer.add(obj1).add(obj2).add(obj3);
     * @endcode
     */
    EmbeddedViewer& add(std::shared_ptr<Object> object);

    /**
     * @brief Add object to a specific layer (fluent API)
     *
     * @param object Object to add
     * @param layer_id Layer identifier
     * @return Reference to this viewer (for chaining)
     */
    EmbeddedViewer& add(std::shared_ptr<Object> object, const std::string& layer_id);

    /**
     * @brief Replace all objects in a layer (fluent API)
     *
     * Clears existing objects in the layer, then adds all new objects atomically.
     *
     * @param objects Vector of objects to set
     * @param layer_id Layer identifier
     * @return Reference to this viewer (for chaining)
     *
     * @example
     * @code
     * std::vector<std::shared_ptr<Object>> objs = {obj1, obj2, obj3};
     * embeddedViewer.setLayer(objs, "my_layer");
     * @endcode
     */
    EmbeddedViewer& setLayer(const std::vector<std::shared_ptr<Object>>& objects,
                              const std::string& layer_id = "default");

    /**
     * @brief Quick add: sphere (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param radius Sphere radius
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    EmbeddedViewer& addSphere(const std::string& id, const Vec3& color, double radius,
                              const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Quick add: box (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param width Box width (X-axis)
     * @param height Box height (Z-axis)
     * @param depth Box depth (Y-axis)
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    EmbeddedViewer& addBox(const std::string& id, const Vec3& color, double width, double height, double depth,
                           const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Quick add: cylinder (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param radius Cylinder radius
     * @param height Cylinder height (along Z-axis)
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    EmbeddedViewer& addCylinder(const std::string& id, const Vec3& color, double radius, double height,
                                const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Get the container widget for embedding in user's UI
     * @return Pointer to the container widget (QWidget*)
     *
     * @note The returned widget can be embedded in other Qt layouts.
     *       The widget's lifetime is managed by EmbeddedViewer.
     */
    QWidget* widget() const;

    /**
     * @brief Export selected object IDs from the current view
     * @return Vector of selected object IDs
     *
     * @note Only exports objects selected in the currently active view.
     *       Returns an empty vector if no view is active or no objects are selected.
     */
    std::vector<std::string> exportSelectedObjectIds() const;

    // Container-level recording controls.
    bool startRecording(const RecordingOptions& options);
    bool pauseRecording();
    bool resumeRecording();
    bool stopRecording();
    bool isRecording() const;
    bool isRecordingPaused() const;
    std::string getLastRecordingError() const;

    /**
     * @brief Destructor
     */
    ~EmbeddedViewer();

    // Disable copy (use move semantics if needed)
    EmbeddedViewer(const EmbeddedViewer&) = delete;
    EmbeddedViewer& operator=(const EmbeddedViewer&) = delete;

    // Enable move
    EmbeddedViewer(EmbeddedViewer&&) noexcept;
    EmbeddedViewer& operator=(EmbeddedViewer&&) noexcept;

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    EmbeddedViewer();
    friend class OctoFlexViewer;
};

/**
 * @brief Simplified facade for OctoFlexView library
 *
 * Provides zero-configuration initialization and fluent API for quick prototyping.
 * Automatically manages Qt Application, Container, ObjectManager, and initial View.
 *
 * @example Basic usage:
 * @code
 * auto viewer = OctoFlexViewer::create("My Viewer");
 * viewer.addSphere("sphere1", Vec3(1, 0, 0), 1.0);
 * viewer.show();
 * return viewer.run();
 * @endcode
 */
class OCTO_FLEX_VIEW_API OctoFlexViewer {
   public:
    /**
     * @brief Callback function type for scene setup
     * Called after OpenGL context is ready
     */
    using SetupCallback = std::function<void(OctoFlexViewer&)>;

    /**
     * @brief Create embedded viewer for integration into existing Qt applications
     *
     * @param parent Parent widget (can be nullptr for top-level widget)
     * @return Embedded viewer instance that can be embedded into user's Qt windows
     *
     * @note This mode does NOT create a QMainWindow or manage the event loop.
     * @note User must have already created QApplication before calling this.
     * @note The returned viewer hides internal ObjectManager - use add() methods instead.
     *
     * @example
     * @code
     * QApplication app(argc, argv);
     * QMainWindow* myWindow = new QMainWindow();
     *
     * auto embeddedViewer = OctoFlexViewer::createEmbedded(myWindow);
     * embeddedViewer.addSphere("sphere", Vec3(1, 0, 0), 1.0);
     *
     * myWindow->setCentralWidget(embeddedViewer.widget());
     * myWindow->show();
     * return app.exec();
     * @endcode
     */
    static EmbeddedViewer createEmbedded(QWidget* parent = nullptr);

    /**
     * @brief Create and initialize a viewer window with automatic Qt Application setup
     *
     * @param title Window title (default: "OctoFlexView")
     * @param width Window width (default: 1024)
     * @param height Window height (default: 768)
     * @param argc Qt argc (optional, uses dummy if not provided)
     * @param argv Qt argv (optional, uses dummy if not provided)
     * @return Initialized viewer instance
     *
     * @note If QApplication already exists, it will be reused.
     * @note The viewer takes ownership of Qt application only if it creates one.
     */
    static OctoFlexViewer create(const std::string& title = "OctoFlexView", int width = 1024, int height = 768,
                                  int* argc = nullptr, char*** argv = nullptr);

    /**
     * @brief Run the viewer application (blocking event loop)
     *
     * @param setup Optional callback for scene setup after OpenGL context is ready
     * @return Application exit code
     *
     * @note The setup callback is useful for operations that require OpenGL context,
     *       such as loading textures.
     */
    int run(SetupCallback setup = nullptr);

    /**
     * @brief Add object to the default layer (fluent API)
     *
     * @param object Object to add
     * @return Reference to this viewer (for chaining)
     *
     * @example
     * @code
     * viewer.add(obj1).add(obj2).add(obj3);
     * @endcode
     */
    OctoFlexViewer& add(std::shared_ptr<Object> object);

    /**
     * @brief Add object to a specific layer (fluent API)
     *
     * @param object Object to add
     * @param layer_id Layer identifier
     * @return Reference to this viewer (for chaining)
     */
    OctoFlexViewer& add(std::shared_ptr<Object> object, const std::string& layer_id);

    /**
     * @brief Replace all objects in a layer (fluent API)
     *
     * Clears existing objects in the layer, then adds all new objects atomically.
     *
     * @param objects Vector of objects to set
     * @param layer_id Layer identifier
     * @return Reference to this viewer (for chaining)
     *
     * @example
     * @code
     * std::vector<std::shared_ptr<Object>> objs = {obj1, obj2, obj3};
     * viewer.setLayer(objs, "my_layer");
     * @endcode
     */
    OctoFlexViewer& setLayer(const std::vector<std::shared_ptr<Object>>& objects,
                              const std::string& layer_id = "default");

    /**
     * @brief Quick add: sphere (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param radius Sphere radius
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    OctoFlexViewer& addSphere(const std::string& id, const Vec3& color, double radius,
                              const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Quick add: box (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param width Box width (X-axis)
     * @param height Box height (Z-axis)
     * @param depth Box depth (Y-axis)
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    OctoFlexViewer& addBox(const std::string& id, const Vec3& color, double width, double height, double depth,
                           const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Quick add: cylinder (fluent API)
     *
     * @param id Object ID
     * @param color RGB color (each component in range [0, 1])
     * @param radius Cylinder radius
     * @param height Cylinder height (along Z-axis)
     * @param position World position (default: origin)
     * @return Reference to this viewer (for chaining)
     */
    OctoFlexViewer& addCylinder(const std::string& id, const Vec3& color, double radius, double height,
                                const Vec3& position = Vec3(0, 0, 0));

    /**
     * @brief Get object manager for advanced operations
     * @return Shared pointer to object manager
     */
    std::shared_ptr<ObjectManager> objectManager() const;

    /**
     * @brief Get container widget for embedding in other UIs
     * @return Pointer to the container widget (QWidget*)
     *
     * @note The returned widget can be embedded in other Qt layouts.
     *       For advanced view management, use the viewer's API methods instead.
     */
    QWidget* container() const;

    /**
     * @brief Show the window
     */
    void show();

    /**
     * @brief Set window title
     * @param title New window title
     */
    void setTitle(const std::string& title);

    /**
     * @brief Resize window
     * @param width New width
     * @param height New height
     */
    void resize(int width, int height);

    /**
     * @brief Export selected object IDs from the current view
     * @return Vector of selected object IDs
     *
     * @note Only exports objects selected in the currently active view.
     *       Returns an empty vector if no view is active or no objects are selected.
     */
    std::vector<std::string> exportSelectedObjectIds() const;

    // Container-level recording controls.
    bool startRecording(const RecordingOptions& options);
    bool pauseRecording();
    bool resumeRecording();
    bool stopRecording();
    bool isRecording() const;
    bool isRecordingPaused() const;
    std::string getLastRecordingError() const;

    /**
     * @brief Destructor
     */
    ~OctoFlexViewer();

    // Disable copy (use move semantics if needed)
    OctoFlexViewer(const OctoFlexViewer&) = delete;
    OctoFlexViewer& operator=(const OctoFlexViewer&) = delete;

    // Enable move
    OctoFlexViewer(OctoFlexViewer&&) noexcept;
    OctoFlexViewer& operator=(OctoFlexViewer&&) noexcept;

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    OctoFlexViewer();
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_VIEWER_H
