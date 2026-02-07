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


#ifndef OCTO_FLEX_VIEW_H
#define OCTO_FLEX_VIEW_H


#include <GL/glew.h>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QRubberBand>
#include <QTime>
#include <QTimer>
#include <QWheelEvent>
#include <QWidget>
#include <glm/glm.hpp>
#include <map>
#include <set>
#include <tuple>
#include <vector>
#include "camera.h"
#include "coordinate_system.h"
#include "info_panel.h"
#include "object.h"
#include "object_manager.h"
#include "object_tree_dialog.h"

namespace octo_flex {

// Render mode enum.
enum class RenderMode {
    RENDER,  // Normal render mode.
    SELECT   // Selection mode.
};

// Selection mode enum.
enum class SelectionMode {
    POINT,  // Point selection.
    RECT    // Rectangle selection.
};

class OctoFlexView : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    OctoFlexView(QWidget* parent = nullptr);
    ~OctoFlexView() override;

    // Initialize OpenGL rendering resources
    bool initialize();

    // Render a specific object
    virtual void renderObject(const Object& object, RenderMode mode = RenderMode::RENDER, GLuint nameID = 0,
                              bool transparent = false);

    // Set view and projection matrices
    void setViewMatrix(const glm::mat4& viewMatrix_);
    void setProjectionMatrix(const glm::mat4& projectionMatrix_);
    virtual void setObjectManager(ObjectManager::Ptr obj_mgr);

    // Refresh rate control
    void setRefreshRate(int fps);
    int refreshRate() const;
    void startRefresh();
    void stopRefresh();
    bool isRefreshing() const;

    // Set object list widget.
    void setObjectListWidget(QListWidget* list);

    // Refresh object list.
    void updateObjectList();

    // Select object.
    void selectObject(const std::string& objId, bool selected);

    // Get selected object IDs.
    const std::set<std::string>& getSelectedObjects() const;

    // Clear all selections.
    void clearSelection();

    // Get camera.
    Camera::Ptr getCamera() const;

    // Copy camera settings from another camera.
    void copyCamera(const Camera::Ptr& sourceCamera);

    // Show object tree dialog.
    void showObjectTreeDialog(bool layerOnly = false);

    // Show selectable layers dialog.
    void showSelectableLayerDialog();

    // Show visible layers dialog.
    void showVisibleLayerDialog();

    // Info panel helpers.
    void setInfoItem(const std::string& id, const std::string& info);
    void setWarningItem(const std::string& id, const std::string& info);
    void setErrorItem(const std::string& id, const std::string& info);
    void removeInfoItem(const std::string& id);
    void clearInfoItems();
    void toggleInfoPanel();
    bool isInfoPanelVisible() const;
    InfoPanel* getInfoPanel() const;

    // Get hidden layers.
    const std::set<std::string>& getUnvisableLayers() const;
    // Set hidden layers.
    void setUnvisableLayers(const std::set<std::string>& layers);

    // Get unselectable layers.
    const std::set<std::string>& getUnselectableLayers() const;
    // Set unselectable layers.
    void setUnselectableLayers(const std::set<std::string>& layers);

    // Get current projection mode.
    bool isPerspectiveMode() const;
    // Set projection mode.
    void setPerspectiveMode(bool isPerspective);

    // Ensure the view regains focus.
    void ensureFocus();

    // Set view ID.
    void setViewId(const std::string& id);
    std::string getViewId() const;

    // Update FPS info.
    void updateFpsInfo();

    // Expand the view.
    void expandView();

    // Collapse the view.
    void collapseView();

    // Get view expanded state.
    bool isExpanded() const;

    // Get grid visibility.
    bool isGridVisible() const;

    // Set whether this is the only view.
    void setIsOnlyView(bool isOnly);

    // Update button states.
    void updateButtonStates();

    // Toggle grid visibility.
    void toggleGrid();

    // Render XYZ axes at origin.
    void renderCoordinateSystem();

    // Coordinate system management.
    // Set the active coordinate system for camera transformations.
    void setCoordinateSystem(CoordinateSystem::Ptr coordSys);
    // Get the active coordinate system.
    CoordinateSystem::Ptr getCoordinateSystem() const;
    // Switch to global (world) coordinate system.
    void useGlobalCoordinateSystem();
    // Switch to local coordinate system from an object.
    void useObjectCoordinateSystem(const std::string& objectId);
    // Get the current coordinate system type (Global/Local).
    CoordinateSystemType getCoordinateSystemType() const;
    // Update coordinate system from attached object (call each frame).
    void updateCoordinateSystem();

   signals:
    // View split/remove signals.
    void requestHorizontalSplit();
    void requestVerticalSplit();
    void requestViewRemove();

    // View expand/collapse signals.
    void requestExpand();
    void requestCollapse();

   protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    // Context menu event handler.
    void contextMenuEvent(QContextMenuEvent* event) override;

    // Mouse event handlers.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    // Keyboard event handlers.
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    // Focus event handlers.
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

    // Event filter.
    bool eventFilter(QObject* obj, QEvent* event) override;

    ObjectManager::Ptr obj_mgr_;
    Camera::Ptr camera_;

   private:
    // Vertex Buffer Objects
    GLuint VBO_;

    // Matrices
    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;

    // Setup fixed-function pipeline
    void setupFixedFunctionPipeline();

    // Render a single shape.
    void renderShape(const Shape& shape, RenderMode mode = RenderMode::RENDER);

    // Draw object info text.
    void drawObjectInfoText();

    // Calculate object info text positions.
    void calculateObjectInfoPosition(const Object& object, bool transparent);

    // Toggle projection mode.
    void toggleProjection();

    // Common GL_SELECT selection handler.
    void handleSelectionWithGLSelect(const QPoint& point, int width, int height, SelectionMode mode);

    // Set selection buffer.
    void setupSelectBuffer(GLuint* buffer, GLint size);

    // Enter selection mode.
    void enterSelectMode(int x, int y, int width, int height);

    // Exit selection mode and process results.
    std::vector<GLuint> processHits(GLint hits, SelectionMode mode);

    // Create context menu.
    void createContextMenu(const QPoint& pos);

    // Draw info panel.
    void drawInfoPanel();

    // Render XY-plane grid.
    void renderGrid();

    // Handle camera movement.
    void handleCameraMovement(float deltaForward, float deltaRight, float deltaUp);

    // Set camera view (relative to selected object or origin).
    void setCameraView(const std::string& viewDirection);

   private:
    QTimer* refreshTimer_;
    int refreshRate_;

    // FPS tracking.
    int frameCount_;
    QTimer* fpsTimer_;
    float currentFps_;
    std::string viewId_;

    // Directly use internal matrices.
    glm::mat4 perspectiveMatrix_;
    glm::mat4 orthoMatrix_;
    bool isPerspective_ = true;

    // Object selection.
    QListWidget* objectListWidget_ = nullptr;
    std::set<std::string> selectedObjects_;

    std::set<std::string> unvisable_layers_;
    std::set<std::string> unselectable_layers_;

    // Rectangle selection.
    QRubberBand rubberBand_;
    QPoint rubberBandOrigin_;
    bool isClick_ = false;

    // GL_SELECT mode.
    static const GLsizei SELECT_BUFFER_SIZE_ = 1024;
    GLuint selectBuffer_[SELECT_BUFFER_SIZE_];
    std::map<GLuint, std::string> nameToObjectId_;

    Vec3 bk_color_;

    // Camera state.
    bool isRotating_ = false;
    bool isPanning_ = false;
    QPoint lastMousePos_;

    // Keyboard state.
    bool keyW_ = false;
    bool keyA_ = false;
    bool keyS_ = false;
    bool keyD_ = false;
    bool keyQ_ = false;
    bool keyE_ = false;

    // Focus state.
    bool hasFocus_ = false;

    // Expand state.
    bool isExpanded_ = false;

    // Whether this is the only view.
    bool isOnlyView_ = false;

    // Object info rendering.
    struct ObjectInfoText {
        std::string text;
        Vec3 color;
        int x;
        int y;
    };
    std::vector<ObjectInfoText> objectInfoToRender_;

    // Context menu.
    QMenu* contextMenu_ = nullptr;

    // Info panel.
    InfoPanel* infoPanel_ = nullptr;

    QPushButton* horizontalSplitButton_ = nullptr;
    QPushButton* verticalSplitButton_ = nullptr;
    QPushButton* removeViewButton_ = nullptr;
    QPushButton* expandButton_ = nullptr;
    QPushButton* gridButton_ = nullptr;

    // Grid visibility.
    bool showGrid_ = false;

    float currentSpeed_ = 0.0f;  // Current movement speed.
    float acceleration_ = 8.f;   // Acceleration per second.
    float maxSpeed_ = 50.0f;     // Max movement speed.
    float deceleration_ = 1.0f;  // Deceleration per second.
    QTime lastKeyPressTime_;     // Time of last key press.
    float view_far_ = 1000;
};

}  // namespace octo_flex
#endif /* OCTO_FLEX_VIEW_H */
