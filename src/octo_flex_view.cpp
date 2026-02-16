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


#include "octo_flex_view.h"
#include <GL/glu.h>  // Add GLU header for gluPickMatrix and gluPerspective
#include <QCursor>   // Add QCursor header for getting mouse position
#include <QMenu>
#include <QOpenGLFunctions>
#include <QPainter>   // Add QPainter header for drawing text
#include <QtDebug>    // Correct to proper Qt header format
#include <algorithm>  // For sorting
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <limits>  // For std::numeric_limits

#include "textured_quad.h"
#include "utils.h"

namespace octo_flex {

OctoFlexView::OctoFlexView(QWidget* parent)
    : QOpenGLWidget(parent),
      VBO_(0),
      viewMatrix_(1.0f),
      projectionMatrix_(1.0f),
      refreshTimer_(new QTimer(this)),
      refreshRate_(30),
      camera_(std::make_shared<Camera>()),
      bk_color_(1., 1., 1.),
      // bk_color_(.0, .0, 0., 1.),
      rubberBand_(QRubberBand::Rectangle, this),
      perspectiveMatrix_(1.0f),
      orthoMatrix_(1.0f),
      frameCount_(0),
      fpsTimer_(new QTimer(this)),
      currentFps_(0.0f),
      viewId_("Unnamed View"),
      isExpanded_(false),
      isOnlyView_(false),
      showGrid_(true) {  // Show grid by default
    // Connect timer to update slot
    connect(refreshTimer_, &QTimer::timeout, this, [this]() {
        frameCount_++;
        update();
    });

    // Connect FPS timer to update slot
    connect(fpsTimer_, &QTimer::timeout, this, &OctoFlexView::updateFpsInfo);
    fpsTimer_->start(1000);  // Update FPS every second

    // Enable right-click context menu
    setContextMenuPolicy(Qt::DefaultContextMenu);

    // Enable mouse tracking to receive mouse move events
    setMouseTracking(true);

    // Set focus policy to allow widget to gain focus by clicking
    setFocusPolicy(Qt::StrongFocus);

    // Install event filter
    installEventFilter(this);

    // Set initial camera position to (10,10,10)
    camera_->setPosition(glm::vec3(5.0f, 5.0f, 5.0f));

    // Make camera look at origin (0,0,0)
    camera_->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    // Create info panel
    infoPanel_ = new InfoPanel(this);

    // Start timer with default refresh rate
    setRefreshRate(refreshRate_);

    // Create expand/collapse button
    expandButton_ = new QPushButton(this);
    expandButton_->setText("⛶");  // Expand icon
    expandButton_->setToolTip("Expand View");
    expandButton_->setFixedSize(20, 20);
    expandButton_->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 150); border: none; border-radius: 3px; }");
    expandButton_->setFocusPolicy(Qt::NoFocus);  // Prevent button from gaining focus
    expandButton_->installEventFilter(this);     // Install event filter
    expandButton_->move(infoPanel_->getMargin() + 75, infoPanel_->getMargin());

    // Connect signals
    connect(expandButton_, &QPushButton::clicked, this, [this]() {
        if (isExpanded_) {
            emit requestCollapse();
            collapseView();
        } else {
            emit requestExpand();
            expandView();
        }
        ensureFocus();
    });

    // Create horizontal split button
    horizontalSplitButton_ = new QPushButton(this);
    horizontalSplitButton_->setText("◫");
    horizontalSplitButton_->setToolTip("Horizontal Split View (Left/Right)");
    horizontalSplitButton_->setFixedSize(20, 20);
    horizontalSplitButton_->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 150); border: none; border-radius: 3px; }");
    horizontalSplitButton_->setFocusPolicy(Qt::NoFocus);  // Prevent button from gaining focus
    horizontalSplitButton_->installEventFilter(this);     // Install event filter
    horizontalSplitButton_->move(infoPanel_->getMargin() + 25, infoPanel_->getMargin());

    // Connect signals, ensure view regains focus after emitting requestHorizontalSplit signal
    connect(horizontalSplitButton_, &QPushButton::clicked, this, [this]() {
        emit requestHorizontalSplit();
        ensureFocus();
    });

    horizontalSplitButton_->hide();  // Hide initially

    // Create vertical split button
    verticalSplitButton_ = new QPushButton(this);
    verticalSplitButton_->setText("⬓");
    verticalSplitButton_->setToolTip("Vertical Split View (Top/Bottom)");
    verticalSplitButton_->setFixedSize(20, 20);
    verticalSplitButton_->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 150); border: none; border-radius: 3px; }");
    verticalSplitButton_->setFocusPolicy(Qt::NoFocus);  // Prevent button from gaining focus
    verticalSplitButton_->installEventFilter(this);     // Install event filter
    verticalSplitButton_->move(infoPanel_->getMargin() + 50, infoPanel_->getMargin());

    // Connect signals, ensure view regains focus after emitting requestVerticalSplit signal
    connect(verticalSplitButton_, &QPushButton::clicked, this, [this]() {
        emit requestVerticalSplit();
        ensureFocus();
    });

    verticalSplitButton_->hide();  // Hide initially

    // Create grid display button
    gridButton_ = new QPushButton(this);
    gridButton_->setText("□");  // Use 'minus grid' icon when displayed
    gridButton_->setToolTip("Hide Grid");
    gridButton_->setFixedSize(20, 20);
    gridButton_->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 150); border: none; border-radius: 3px; }");
    gridButton_->setFocusPolicy(Qt::NoFocus);  // Prevent button from gaining focus
    gridButton_->installEventFilter(this);     // Install event filter
    gridButton_->move(infoPanel_->getMargin() + 100, infoPanel_->getMargin());

    // Connect signals, ensure view regains focus after toggling grid display state
    connect(gridButton_, &QPushButton::clicked, this, [this]() {
        toggleGrid();
        ensureFocus();
    });

    // Create remove view button
    removeViewButton_ = new QPushButton(this);
    removeViewButton_->setText("✕");
    removeViewButton_->setToolTip("Remove Current View");
    removeViewButton_->setFixedSize(20, 20);
    removeViewButton_->setStyleSheet(
        "QPushButton { background-color: rgba(255, 255, 255, 150); border: none; border-radius: 3px; }");
    removeViewButton_->setFocusPolicy(Qt::NoFocus);  // Prevent button from gaining focus
    removeViewButton_->installEventFilter(this);     // Install event filter
    removeViewButton_->move(infoPanel_->getMargin() + 125, infoPanel_->getMargin());

    // Connect signals, ensure view regains focus after emitting requestViewRemove signal
    connect(removeViewButton_, &QPushButton::clicked, this, [this]() {
        emit requestViewRemove();
        ensureFocus();
    });

    removeViewButton_->hide();  // Hide initially

    // After all buttons are created, connect infoPanel signals
    if (infoPanel_ && infoPanel_->getToggleButton()) {
        // Connect signals, ensure view regains focus after calling toggleInfoPanel
        connect(infoPanel_->getToggleButton(), &QPushButton::clicked, this, [this]() {
            toggleInfoPanel();
            ensureFocus();
        });
    }

    // Update view matrix
    viewMatrix_ = camera_->getViewMatrix();

    // Output debug information
    std::cout << "OctoFlexView constructor: Setting focus policy to StrongFocus" << std::endl;
    std::cout << "Initial camera position: (10, 10, 10), looking at: (0, 0, 0)" << std::endl;

    // Initially add some test info items
    setInfoItem("view_id", "View ID: " + viewId_);
    setInfoItem("fps", "FPS: 0.0");
    setInfoItem("refresh_rate", "Target Refresh Rate: " + std::to_string(refreshRate_) + " FPS");
    setInfoItem("normal_msg", "Normal Info - Black Display");
    setWarningItem("warning_msg", "Warning Info - Orange Display");
    setErrorItem("error_msg", "Error Info - Red Display");

    // Add grid status info
    if (showGrid_) {
        setInfoItem("grid_state", "Grid: Shown");
    }
}

OctoFlexView::~OctoFlexView() {
    // Stop the refresh timer
    if (refreshTimer_->isActive()) {
        refreshTimer_->stop();
    }

    // Stop the FPS timer
    if (fpsTimer_->isActive()) {
        fpsTimer_->stop();
    }

    // Release button resources
    if (infoPanel_) {
        delete infoPanel_;
        infoPanel_ = nullptr;
    }

    if (horizontalSplitButton_) {
        delete horizontalSplitButton_;
        horizontalSplitButton_ = nullptr;
    }

    if (verticalSplitButton_) {
        delete verticalSplitButton_;
        verticalSplitButton_ = nullptr;
    }

    if (removeViewButton_) {
        delete removeViewButton_;
        removeViewButton_ = nullptr;
    }

    if (expandButton_) {
        delete expandButton_;
        expandButton_ = nullptr;
    }

    if (gridButton_) {
        delete gridButton_;
        gridButton_ = nullptr;
    }

    // Clean up OpenGL resources
    makeCurrent();
    glDeleteBuffers(1, &VBO_);
    doneCurrent();
}

bool OctoFlexView::initialize() {
    // This method is called to initialize OpenGL resources
    // The actual initialization happens in initializeGL()
    return true;
}

void OctoFlexView::initializeGL() {
    // Initialize OpenGL functions
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(bk_color_.x, bk_color_.y, bk_color_.z, 1.0);
    glDepthMask(GL_TRUE);

    // Generate VBO
    glGenBuffers(1, &VBO_);

    // Update view matrix
    viewMatrix_ = camera_->getViewMatrix();
}

void OctoFlexView::paintGL() {
    // Update coordinate system from attached object (if any)
    updateCoordinateSystem();

    // Clear previous frame's object info list
    objectInfoToRender_.clear();

    // Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bk_color_.x, bk_color_.y, bk_color_.z, 1.0);

    // Use fixed-function pipeline
    setupFixedFunctionPipeline();

    // If grid is enabled, render XY plane grid
    if (showGrid_) {
        renderGrid();
    }

    // Draw coordinate system at origin
    renderCoordinateSystem();

    // Render all objects in the object manager
    if (obj_mgr_ == nullptr) return;

    // First render: render all opaque shapes
    auto layers = obj_mgr_->layers();
    for (const auto& [layer_id, layer] : layers) {
        if (unvisable_layers_.count(layer_id) > 0) continue;
        auto objects = layer->objects();
        for (const auto& [obj_id, object] : objects) {
            renderObject(*object, RenderMode::RENDER, 0, false);
        }
    }

    // Second render: render all transparent shapes
    glDepthMask(GL_FALSE);
    for (const auto& [layer_id, layer] : layers) {
        if (unvisable_layers_.count(layer_id) > 0) continue;
        auto objects = layer->objects();
        for (const auto& [obj_id, object] : objects) {
            renderObject(*object, RenderMode::RENDER, 0, true);
        }
    }
    glDepthMask(GL_TRUE);

    // Reset OpenGL state before using QPainter, to prevent TexturedQuad from affecting text rendering
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Key: restore pixel alignment to OpenGL default values, TexturedQuad changes this setting causing text
    // misalignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    // Use QPainter to draw object info text
    if (!objectInfoToRender_.empty()) {
        drawObjectInfoText();
    }

    // Draw info panel
    if (infoPanel_) {
        bool isVisible = infoPanel_->isVisible();
        // std::cout << "OctoFlexView::paintGL - info panel state: " << (isVisible ? "shown" : "hidden")
        //           << std::endl;

        if (isVisible) {
            QPainter painter(this);
            // std::cout << "OctoFlexView::paintGL - Creating QPainter to draw info panel" << std::endl;
            infoPanel_->draw(painter);
            // std::cout << "OctoFlexView::paintGL - Info panel drawing completed" << std::endl;
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Clear outdated objects after rendering (deferred deletion)
    if (obj_mgr_) {
        obj_mgr_->clearOutdatedObjects();
    }
}

// Get camera.
Camera::Ptr OctoFlexView::getCamera() const { return camera_; }

// Copy settings from another camera.
void OctoFlexView::copyCamera(const Camera::Ptr& sourceCamera) {
    if (sourceCamera) {
        // 1. Copy camera position.
        camera_->setPosition(sourceCamera->getPosition());

        // 2. Copy camera speed.
        camera_->setSpeed(sourceCamera->getSpeed());

        // 3. Copy camera vectors to keep directions consistent.
        camera_->setVectors(sourceCamera->getFront(), sourceCamera->getUp(), sourceCamera->getRight());

        // 4. Update view matrix.
        viewMatrix_ = camera_->getViewMatrix();

        // Request repaint.
        update();
    }
}

void OctoFlexView::setupFixedFunctionPipeline() {
    // Setup projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Convert glm matrix to OpenGL matrix
    GLfloat projMatrix[16];
    const float* projSource = glm::value_ptr(projectionMatrix_);
    for (int i = 0; i < 16; i++) {
        projMatrix[i] = projSource[i];
    }
    glLoadMatrixf(projMatrix);

    // Setup modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Update view matrix.
    viewMatrix_ = camera_->getViewMatrix();

    // Convert glm matrix to OpenGL matrix
    GLfloat viewMatrix[16];
    const float* viewSource = glm::value_ptr(this->viewMatrix_);
    for (int i = 0; i < 16; i++) {
        viewMatrix[i] = viewSource[i];
    }
    glLoadMatrixf(viewMatrix);

    /*
    // Enable two-sided lighting.
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    */

    // Enable vertex and color arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

void OctoFlexView::renderObject(const Object& object, RenderMode mode, GLuint nameID, bool transparent) {
    // If in selection mode, set the object name ID.
    if (mode == RenderMode::SELECT) {
        glLoadName(nameID);
    }

    // Check if the object is selected (render mode only).
    bool isSelected = selectedObjects_.count(object.id()) > 0;

    // In opaque render pass, compute and store info text position for selected objects.
    if (mode == RenderMode::RENDER && isSelected && !transparent) {
        calculateObjectInfoPosition(object, transparent);
    }

    // Render all shapes.
    for (const auto& shape : object.shapes()) {
        // Filter by transparency; all shapes respect transparency parameter.
        bool isShapeTransparent = (shape->transparency() < 0.99);

        // In selection mode, render all shapes regardless of transparency.
        // In render mode, render by transparency flag.
        if (mode == RenderMode::SELECT || isShapeTransparent == transparent) {
            renderShape(*shape, mode);
        }
    }
}

// Compute display position for object info text.
void OctoFlexView::calculateObjectInfoPosition(const Object& object, bool transparent) {
    // Only compute during opaque render pass.
    if (transparent) return;

    // Get current modelview and projection matrices.
    GLdouble modelMatrix[16], projMatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Find the highest screen-space point (max Y in OpenGL screen coords).
    double maxScreenY = -std::numeric_limits<double>::max();
    double correspondingScreenX = 0;
    double correspondingScreenZ = 0;

    // Walk all points in the object.
    for (const auto& shape : object.shapes()) {
        for (const auto& point : shape->points()) {
            // Project 3D point to screen space.
            GLdouble screenX, screenY, screenZ;
            gluProject(point.x, point.y, point.z, modelMatrix, projMatrix, viewport, &screenX, &screenY, &screenZ);

            // Update highest point (max Y in OpenGL screen coords).
            if (screenY > maxScreenY) {
                maxScreenY = screenY;
                correspondingScreenX = screenX;
                correspondingScreenZ = screenZ;
            }
        }
    }

    // Show text only if the point is inside the view (Z in [0, 1]).
    if (correspondingScreenZ >= 0.0 && correspondingScreenZ <= 1.0) {
        // Store object info and screen coordinates to draw after paintGL.
        // Note: Qt Y grows downward while OpenGL Y grows upward, so flip Y.
        // Offset slightly above the highest point for readability.
        const int TEXT_OFFSET_Y = 15;  // Upward pixel offset.
        objectInfoToRender_.push_back({object.info(), object.textColor(), static_cast<int>(correspondingScreenX),
                                       static_cast<int>(viewport[3] - maxScreenY - TEXT_OFFSET_Y)});
    }
}

// Render a single shape.
void OctoFlexView::renderShape(const Shape& shape, RenderMode mode) {
    const auto& points = shape.points();

    if (points.empty()) return;

    if (mode == RenderMode::RENDER) {
        const auto* textured = dynamic_cast<const TexturedQuad*>(&shape);
        if (textured && textured->hasTexture() && points.size() >= 4) {
            const auto& uvs = textured->uvs();

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textured->textureId());
            glColor4f(1.0f, 1.0f, 1.0f, shape.transparency());

            glBegin(GL_QUADS);
            for (size_t i = 0; i < 4; ++i) {
                glTexCoord2f(uvs[i].u, uvs[i].v);
                glVertex3f(points[i].x, points[i].y, points[i].z);
            }
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            return;
        }
    }

    // Pick draw mode based on shape type.
    GLenum glMode;
    switch (shape.type()) {
        case Shape::Points:
            glMode = GL_POINTS;
            break;
        case Shape::Lines:
            glMode = GL_LINES;
            break;
        case Shape::Dash:
            // Dashed lines are treated as solid in selection mode.
            if (mode == RenderMode::SELECT) {
                glMode = GL_LINES;
            } else {
                // Use dashed line mode in render mode.
                glMode = GL_LINES;
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0x00FF);  // Dashed pattern.
            }
            break;
        case Shape::Loop:
            glMode = GL_LINE_LOOP;
            break;
        case Shape::Polygon:
            glMode = GL_POLYGON;
            break;
        case Shape::TexturedQuad:
            glMode = GL_QUADS;
            break;
        default:
            glMode = GL_POINTS;
    }

    // Set line width for line types.
    if (shape.type() == Shape::Lines || shape.type() == Shape::Dash || shape.type() == Shape::Loop) {
        glLineWidth(shape.width());
    }

    // Set point size for point type.
    if (shape.type() == Shape::Points) {
        glPointSize(shape.width());
    }

    // Begin drawing.
    glBegin(glMode);
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& point = points[i];
        const auto& color = shape.color(i);
        // Set vertex color.
        if (mode == RenderMode::RENDER) {
            glColor4f(color.x, color.y, color.z, shape.transparency());
        }
        // Set vertex position.
        glVertex3f(point.x, point.y, point.z);
    }
    glEnd();

    // Restore state.
    if (mode == RenderMode::RENDER) {
        if (shape.type() == Shape::Dash) {
            glDisable(GL_LINE_STIPPLE);
        }
    }

    // Restore default line width and point size.
    glLineWidth(1.0f);
    glPointSize(1.0f);
}

void OctoFlexView::resizeGL(int width, int height) {
    // Update viewport.
    glViewport(0, 0, width, height);

    // Update projection matrices.
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Update perspective projection.
    perspectiveMatrix_ = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, view_far_);

    // Update orthographic projection.
    float orthoSize = 5.0f;
    orthoMatrix_ =
        glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, 0.1f, view_far_);

    // Update active projection matrix.
    if (isPerspective_) {
        projectionMatrix_ = perspectiveMatrix_;
    } else {
        projectionMatrix_ = orthoMatrix_;
    }

    // Update button positions.
    if (infoPanel_) {
        infoPanel_->updateButtonPosition();
    }

    horizontalSplitButton_->move(infoPanel_->getMargin() + 25, infoPanel_->getMargin());
    verticalSplitButton_->move(infoPanel_->getMargin() + 50, infoPanel_->getMargin());
    expandButton_->move(infoPanel_->getMargin() + 75, infoPanel_->getMargin());
    gridButton_->move(infoPanel_->getMargin() + 100, infoPanel_->getMargin());
    removeViewButton_->move(infoPanel_->getMargin() + 125, infoPanel_->getMargin());
}

void OctoFlexView::setViewMatrix(const glm::mat4& viewMatrix_) {
    this->viewMatrix_ = viewMatrix_;
    update();  // Request a repaint
}

void OctoFlexView::setProjectionMatrix(const glm::mat4& projectionMatrix_) {
    this->projectionMatrix_ = projectionMatrix_;
    update();  // Request a repaint
}

void OctoFlexView::setObjectManager(ObjectManager::Ptr obj_mgr) {
    obj_mgr_ = obj_mgr;
    updateObjectList();
}

void OctoFlexView::setRefreshRate(int fps) {
    if (fps <= 0) {
        // If fps is 0 or negative, stop the timer
        refreshTimer_->stop();
        refreshRate_ = 0;
    } else {
        // Calculate interval in milliseconds
        int interval = 1000 / fps;
        refreshTimer_->start(interval);
        refreshRate_ = fps;

        // Update target FPS info.
        setInfoItem("refresh_rate", "Target Refresh Rate: " + std::to_string(refreshRate_) + " FPS");
    }
}

int OctoFlexView::refreshRate() const { return refreshRate_; }

void OctoFlexView::startRefresh() {
    if (!refreshTimer_->isActive() && refreshRate_ > 0) {
        int interval = 1000 / refreshRate_;
        refreshTimer_->start(interval);
    }
}

void OctoFlexView::stopRefresh() {
    if (refreshTimer_->isActive()) {
        refreshTimer_->stop();
    }
}

bool OctoFlexView::isRefreshing() const { return refreshTimer_->isActive(); }

// Set object list widget.
void OctoFlexView::setObjectListWidget(QListWidget* list) {
    objectListWidget_ = list;
    updateObjectList();
}

// Update object list.
void OctoFlexView::updateObjectList() {
    if (!objectListWidget_ || !obj_mgr_) return;

    objectListWidget_->clear();

    /*
    for (const auto& obj : obj_mgr_->objects()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(obj->id()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(selectedObjects_.count(obj->id()) > 0 ? Qt::Checked : Qt::Unchecked);
        objectListWidget_->addItem(item);
    }
    */
    for (auto layer : obj_mgr_->layers()) {
        for (auto it : layer.second->objects()) {
            auto obj = it.second;
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(obj->id()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(selectedObjects_.count(obj->id()) > 0 ? Qt::Checked : Qt::Unchecked);
            objectListWidget_->addItem(item);
        }
    }
}

// Select object.
void OctoFlexView::selectObject(const std::string& objId, bool selected) {
    if (selected) {
        selectedObjects_.insert(objId);
    } else {
        selectedObjects_.erase(objId);
    }

    // Update selection state in the list widget.
    if (objectListWidget_) {
        for (int i = 0; i < objectListWidget_->count(); ++i) {
            QListWidgetItem* item = objectListWidget_->item(i);
            if (item->text().toStdString() == objId) {
                item->setCheckState(selected ? Qt::Checked : Qt::Unchecked);
                break;
            }
        }
    }

    update();  // Request repaint.
}

// Get selected object IDs.
const std::set<std::string>& OctoFlexView::getSelectedObjects() const { return selectedObjects_; }

// Clear all selections.
void OctoFlexView::clearSelection() {
    selectedObjects_.clear();

    // Update selection state in the list widget.
    if (objectListWidget_) {
        for (int i = 0; i < objectListWidget_->count(); ++i) {
            objectListWidget_->item(i)->setCheckState(Qt::Unchecked);
        }
    }

    update();  // Request repaint.
}

// Mouse press event.
void OctoFlexView::mousePressEvent(QMouseEvent* event) {
    // Ensure the widget has focus.
    if (!hasFocus_) {
        setFocus();
        std::cout << "OctoFlexView gained focus via mouse click" << std::endl;
    }

    // Check if Ctrl is pressed.
    bool ctrlPressed = (event->modifiers() & Qt::ControlModifier);

    if (event->button() == Qt::LeftButton) {
        if (ctrlPressed) {
            // Ctrl + left click: selection.
            // Record origin.
            rubberBandOrigin_ = event->pos();

            // Show rubber band.
            rubberBand_.setGeometry(QRect(rubberBandOrigin_, QSize()));
            rubberBand_.show();

            // Track click vs drag.
            isClick_ = true;
        } else {
            // Left click: camera pan (was middle button).
            lastMousePos_ = event->pos();
            isPanning_ = true;
            setCursor(Qt::SizeAllCursor);
        }
    } else if (event->button() == Qt::MiddleButton) {
        // Middle click: camera rotate (was left button).
        lastMousePos_ = event->pos();
        isRotating_ = true;
        setCursor(Qt::ClosedHandCursor);
    }

    // Stop event propagation.
    event->accept();
}

// Mouse move event.
void OctoFlexView::mouseMoveEvent(QMouseEvent* event) {
    // Check if Ctrl is pressed.
    bool ctrlPressed = (event->modifiers() & Qt::ControlModifier);

    if ((event->buttons() & Qt::LeftButton) && ctrlPressed) {
        // Ctrl + left drag: rectangle selection.
        // Update rubber band size.
        rubberBand_.setGeometry(QRect(rubberBandOrigin_, event->pos()).normalized());

        // If moved beyond a threshold, treat as drag.
        if ((event->pos() - rubberBandOrigin_).manhattanLength() > 3) {
            isClick_ = false;
        }
    } else if ((event->buttons() & Qt::MiddleButton) && isRotating_) {
        // Spherical camera rotation (was left button).

        // 1. Compute ray direction for previous mouse position.
        float oldX = (2.0f * lastMousePos_.x()) / width() - 1.0f;
        float oldY = 1.0f - (2.0f * lastMousePos_.y()) / height();

        // Create point on near plane.
        glm::vec4 oldRayClip(oldX, oldY, -1.0f, 1.0f);

        // Transform to view space.
        glm::mat4 invProj = glm::inverse(projectionMatrix_);
        glm::vec4 oldRayEye = invProj * oldRayClip;
        oldRayEye = glm::vec4(oldRayEye.x, oldRayEye.y, -1.0f, 0.0f);

        // Transform to world space.
        glm::mat4 invView = glm::inverse(viewMatrix_);
        glm::vec4 oldRayWorld = invView * oldRayEye;
        glm::vec3 oldRayDir = glm::normalize(glm::vec3(oldRayWorld));

        // 2. Compute ray direction for current mouse position.
        float newX = (2.0f * event->pos().x()) / width() - 1.0f;
        float newY = 1.0f - (2.0f * event->pos().y()) / height();

        // Create point on near plane.
        glm::vec4 newRayClip(newX, newY, -1.0f, 1.0f);

        // Transform to view space.
        glm::vec4 newRayEye = invProj * newRayClip;
        newRayEye = glm::vec4(newRayEye.x, newRayEye.y, -1.0f, 0.0f);

        // Transform to world space.
        glm::vec4 newRayWorld = invView * newRayEye;
        glm::vec3 newRayDir = glm::normalize(glm::vec3(newRayWorld));

        // 3. Rotate camera using spherical rotation.
        camera_->rotateSphere(oldRayDir, newRayDir);

        // Update last mouse position.
        lastMousePos_ = event->pos();

        // Update view matrix.
        viewMatrix_ = camera_->getViewMatrix();

        // Request repaint.
        update();
    } else if ((event->buttons() & Qt::LeftButton) && isPanning_) {
        // Left drag: camera pan (was middle button).

        // Compute screen-space delta.
        QPoint delta = event->pos() - lastMousePos_;

        // Adjust pan speed based on view size.
        float speedFactor = camera_->getSpeed() * 0.05f;
        float dx = -delta.x() * speedFactor;
        float dy = delta.y() * speedFactor;  // Invert Y for more intuitive motion.

        // Pan along camera right and up.
        glm::vec3 moveVector = camera_->getRight() * dx + camera_->getUp() * dy;

        // Update camera position.
        camera_->setPosition(camera_->getPosition() + moveVector);

        // Update last mouse position.
        lastMousePos_ = event->pos();

        // Update view matrix.
        viewMatrix_ = camera_->getViewMatrix();

        // Request repaint.
        update();
    }
}

// Mouse release event.
void OctoFlexView::mouseReleaseEvent(QMouseEvent* event) {
    // Check if Ctrl is pressed.
    bool ctrlPressed = (event->modifiers() & Qt::ControlModifier);

    if (event->button() == Qt::LeftButton) {
        if (rubberBand_.isVisible()) {
            // Finish selection.
            // Hide rubber band.
            rubberBand_.hide();

            // Get selection rectangle.
            QRect selectionRect = rubberBand_.geometry();

            // If it's a click (not a drag).
            if (isClick_) {
                // Point selection using GL_SELECT.
                handleSelectionWithGLSelect(event->pos(), 5, 5, SelectionMode::POINT);
            } else {
                // Ensure selection rectangle is valid.
                if (selectionRect.width() < 1 || selectionRect.height() < 1) {
                    std::cout << "Selection rectangle invalid, skipping box select" << std::endl;
                    return;
                }

                // Log selection rectangle.
                std::cout << "Selection rectangle: (" << selectionRect.x() << ", " << selectionRect.y() << ", "
                          << selectionRect.width() << ", " << selectionRect.height() << ")" << std::endl;

                // Box selection using GL_SELECT.
                // Note: pass top-left coordinates and width/height.
                handleSelectionWithGLSelect(QPoint(selectionRect.x(), selectionRect.y()), selectionRect.width(),
                                            selectionRect.height(), SelectionMode::RECT);
            }
        } else if (isPanning_) {
            // Left release: end camera pan (was middle button).
            isPanning_ = false;
            setCursor(Qt::ArrowCursor);
        }
    } else if (event->button() == Qt::MiddleButton) {
        // Middle release: end camera rotation (was left button).
        isRotating_ = false;
        setCursor(Qt::ArrowCursor);
    }
}

// Wheel event.
void OctoFlexView::wheelEvent(QWheelEvent* event) {
    // Get wheel delta.
    QPoint angleDelta = event->angleDelta();
    float deltaForward = 0.0f;

    // Scroll down (negative) moves forward, up (positive) moves backward.
    deltaForward = -angleDelta.y() / 120.0f;  // 120 is Qt's standard wheel step.

    // Ignore if no scroll.
    if (deltaForward == 0.0f) {
        event->accept();
        return;
    }

    // Compute mouse ray direction.
    // Get current mouse position.
    // QPoint mousePos = event->position().toPoint();
    QPoint mousePos = event->pos();

    // Convert to normalized device coordinates (-1 to 1).
    float x = (2.0f * mousePos.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y()) / height();

    // Create point on near plane.
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Transform to view space.
    glm::mat4 invProj = glm::inverse(projectionMatrix_);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Transform to world space.
    glm::mat4 invView = glm::inverse(viewMatrix_);
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

    // Move along ray direction.
    // Scale movement for sensitivity.
    camera_->moveWithRay(deltaForward * camera_->getSpeed() * 2.0f, 0.0f, 0.0f, rayDirection);

    // Update view matrix.
    viewMatrix_ = camera_->getViewMatrix();

    // Request repaint.
    update();

    // Stop event propagation.
    event->accept();
}

// Toggle projection mode.
void OctoFlexView::toggleProjection() {
    isPerspective_ = !isPerspective_;

    if (isPerspective_) {
        projectionMatrix_ = perspectiveMatrix_;
    } else {
        projectionMatrix_ = orthoMatrix_;
    }

    update();  // Request repaint.
}

// Set camera view (relative to selected object or origin).
void OctoFlexView::setCameraView(const std::string& viewDirection) {
    // Determine target position: selected object or origin.
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);
    Quaternion objectOrientation(1.0, 0.0, 0.0, 0.0);  // Identity quaternion (no rotation)
    bool hasObject = false;

    if (!selectedObjects_.empty() && obj_mgr_) {
        // Get the first selected object's position and orientation.
        const std::string& objId = *selectedObjects_.begin();
        auto [layerId, object] = obj_mgr_->findObject(objId);
        if (object) {
            Vec3 objPos = object->position();
            targetPosition = glm::vec3(objPos.x, objPos.y, objPos.z);
            objectOrientation = object->orientation();
            hasObject = true;
        }
    }

    // Calculate camera position based on view direction.
    float distance = 20.0f;  // Distance from target.

    // Local direction vectors in object's coordinate system (Z-up engineering standard):
    // Z axis -> Top/Bottom views
    // X axis -> Front/Back views
    // Y axis -> Left/Right views
    Vec3 localViewDir(0.0, 0.0, 0.0);
    Vec3 localUpDir(0.0, 0.0, 1.0);  // Z is up

    if (viewDirection == "front") {
        // Front view: look from +X towards -X (along X axis)
        localViewDir = Vec3(1.0, 0.0, 0.0);
        localUpDir = Vec3(0.0, 0.0, 1.0);  // Z up
    } else if (viewDirection == "back") {
        // Back view: look from -X towards +X
        localViewDir = Vec3(-1.0, 0.0, 0.0);
        localUpDir = Vec3(0.0, 0.0, 1.0);  // Z up
    } else if (viewDirection == "top") {
        // Top view: look from +Z down towards -Z
        localViewDir = Vec3(0.0, 0.0, 1.0);
        localUpDir = Vec3(0.0, -1.0, 0.0);  // Y points "up" on screen for top view
    } else if (viewDirection == "bottom") {
        // Bottom view: look from -Z up towards +Z
        localViewDir = Vec3(0.0, 0.0, -1.0);
        localUpDir = Vec3(0.0, 1.0, 0.0);  // Y points "up" on screen for bottom view
    } else if (viewDirection == "left") {
        // Left view: look from -Y towards +Y (along Y axis)
        localViewDir = Vec3(0.0, -1.0, 0.0);
        localUpDir = Vec3(0.0, 0.0, 1.0);  // Z up
    } else if (viewDirection == "right") {
        // Right view: look from +Y towards -Y
        localViewDir = Vec3(0.0, 1.0, 0.0);
        localUpDir = Vec3(0.0, 0.0, 1.0);  // Z up
    } else {
        // Default: front view (+X towards -X)
        localViewDir = Vec3(1.0, 0.0, 0.0);
        localUpDir = Vec3(0.0, 0.0, 1.0);
    }

    // Transform local directions to world space if object has orientation.
    glm::vec3 viewDir, upDir;
    if (hasObject) {
        Vec3 rotatedViewDir = quaternionRotateVector(objectOrientation, localViewDir);
        Vec3 rotatedUpDir = quaternionRotateVector(objectOrientation, localUpDir);
        viewDir = glm::vec3(rotatedViewDir.x, rotatedViewDir.y, rotatedViewDir.z);
        upDir = glm::vec3(rotatedUpDir.x, rotatedUpDir.y, rotatedUpDir.z);
    } else {
        // Use world coordinate system (no rotation).
        viewDir = glm::vec3(localViewDir.x, localViewDir.y, localViewDir.z);
        upDir = glm::vec3(localUpDir.x, localUpDir.y, localUpDir.z);
    }

    // Calculate camera position: target + distance * view direction.
    glm::vec3 cameraPosition = targetPosition + viewDir * distance;

    // Set camera position and make it look at target.
    camera_->setPosition(cameraPosition);
    camera_->lookAt(targetPosition);

    // Set camera vectors (front, up, right).
    // Note: right = cross(front, up) for right-hand coordinate system
    glm::vec3 front = glm::normalize(-viewDir);
    glm::vec3 right = glm::normalize(glm::cross(front, upDir));
    camera_->setVectors(front, upDir, right);

    update();  // Request repaint.
}

// Common GL_SELECT selection handler.
void OctoFlexView::handleSelectionWithGLSelect(const QPoint& point, int width, int height, SelectionMode mode) {
    if (!obj_mgr_) return;

    // Ensure OpenGL context is current.
    makeCurrent();

    // Clear name-to-object mapping.
    nameToObjectId_.clear();

    // Set selection buffer.
    setupSelectBuffer(selectBuffer_, SELECT_BUFFER_SIZE_);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Enter selection mode and set pick region.
    enterSelectMode(point.x(), point.y(), width, height);

    // Render all objects, assigning unique name IDs.
    GLuint nameID = 1;
    auto layers = obj_mgr_->layers();
    for (const auto& [layer_id, layer] : layers) {
        if (unvisable_layers_.count(layer_id) > 0) continue;
        if (unselectable_layers_.count(layer_id) > 0) continue;
        auto objects = layer->objects();
        for (const auto& [id, obj] : objects) {
            // Record name ID to object ID mapping.
            nameToObjectId_[nameID] = id;

            // Render object in selection mode.
            renderObject(*obj, RenderMode::SELECT, nameID, false);

            // Increment name ID.
            nameID++;
        }
    }

    // Process hit results.
    GLint hits = glRenderMode(GL_RENDER);
    std::vector<GLuint> selectedNames = processHits(hits, mode);

    // Release OpenGL context.
    doneCurrent();

    // If there are hits, process selection.
    if (!selectedNames.empty()) {
        // Track processed object IDs to avoid duplicates.
        std::set<std::string> processedObjIds;

        for (GLuint name : selectedNames) {
            if (nameToObjectId_.find(name) != nameToObjectId_.end()) {
                std::string objId = nameToObjectId_[name];

                // Skip if already processed.
                if (processedObjIds.count(objId) > 0) {
                    continue;
                }

                // Mark as processed.
                processedObjIds.insert(objId);

                // Handle selection based on mode.
                if (mode == SelectionMode::POINT) {
                    // Point mode: toggle selection.
                    bool isSelected = selectedObjects_.count(objId) > 0;
                    selectObject(objId, !isSelected);
                } else {
                    // Rectangle mode: select object.
                    selectObject(objId, true);
                }
            }
        }
    }

    // Request repaint.
    update();
}

// Enter selection mode.
void OctoFlexView::enterSelectMode(int x, int y, int width, int height) {
    // Get current viewport.
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Switch to selection mode.
    glRenderMode(GL_SELECT);

    // Initialize name stack.
    glInitNames();
    glPushName(0);

    // Save current matrix.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Create pick matrix.
    // Note: OpenGL origin is bottom-left; Qt origin is top-left.
    // Convert y to OpenGL coordinates.
    int glY = viewport[3] - y;

    // Ensure pick region is at least 1x1.
    int pickWidth = std::max(1, width);
    int pickHeight = std::max(1, height);

    // Create pick matrix.
    // Note: gluPickMatrix expects the center, not the top-left.
    // Offset x and glY by half width/height.
    float centerX = x + pickWidth / 2.0f;
    float centerY = glY - pickHeight / 2.0f;

    gluPickMatrix(centerX, centerY, pickWidth, pickHeight, viewport);

    // Apply the same projection matrix used in render mode.
    // Use projectionMatrix_ to stay consistent with setupFixedFunctionPipeline.
    GLfloat projMatrix[16];
    const float* projSource = glm::value_ptr(projectionMatrix_);
    for (int i = 0; i < 16; i++) {
        projMatrix[i] = projSource[i];
    }
    glMultMatrixf(projMatrix);

    // Switch to model-view matrix.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Apply the same view matrix used in render mode.
    // Use viewMatrix_ to stay consistent with setupFixedFunctionPipeline.
    GLfloat viewMatrix[16];
    const float* viewSource = glm::value_ptr(viewMatrix_);
    for (int i = 0; i < 16; i++) {
        viewMatrix[i] = viewSource[i];
    }
    glLoadMatrixf(viewMatrix);
}

// Exit selection mode and process results.
std::vector<GLuint> OctoFlexView::processHits(GLint hits, SelectionMode mode) {
    std::vector<GLuint> selectedNames;

    if (hits <= 0) {
        // No hits.
        return selectedNames;
    }

    // Process hit records.
    GLuint* ptr = selectBuffer_;

    if (mode == SelectionMode::POINT) {
        // Point mode: only return the nearest hit (smallest minZ)
        GLuint nearestName = 0;
        GLuint smallestMinZ = UINT_MAX;

        for (int i = 0; i < hits; i++) {
            GLuint numNames = *ptr;
            ptr++;
            GLuint minZ = *ptr;
            ptr++;
            GLuint maxZ = *ptr;
            ptr++;

            // Check if this hit is nearer than previous
            if (minZ < smallestMinZ) {
                smallestMinZ = minZ;
                if (numNames > 0) {
                    nearestName = *(ptr);  // Get first name in this hit
                }
            }

            // Skip names in this record
            ptr += numNames;
        }

        // Only add the nearest object
        if (nearestName != 0) {
            selectedNames.push_back(nearestName);
        }
    } else {
        // Rectangle mode: collect all hit names (original behavior)
        for (int i = 0; i < hits; i++) {
            GLuint numNames = *ptr;
            ptr++;
            GLuint minZ = *ptr;
            ptr++;
            GLuint maxZ = *ptr;
            ptr++;

            // Collect all hit names.
            for (GLuint j = 0; j < numNames; j++) {
                GLuint name = *ptr;
                selectedNames.push_back(name);
                ptr++;
            }
        }
    }

    // Restore matrices.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Switch back to render mode.
    glRenderMode(GL_RENDER);

    return selectedNames;
}

// Set selection buffer.
void OctoFlexView::setupSelectBuffer(GLuint* buffer, GLint size) { glSelectBuffer(size, buffer); }

// Focus in event.
void OctoFlexView::focusInEvent(QFocusEvent* event) {
    hasFocus_ = true;
    std::cout << "OctoFlexView gained focus" << std::endl;

    // Show buttons on focus; enablement depends on state.
    if (horizontalSplitButton_) horizontalSplitButton_->show();
    if (verticalSplitButton_) verticalSplitButton_->show();
    if (removeViewButton_) removeViewButton_->show();
    if (expandButton_) expandButton_->show();
    if (gridButton_) gridButton_->show();

    // Update button states.
    updateButtonStates();

    QOpenGLWidget::focusInEvent(event);
}

// Focus out event.
void OctoFlexView::focusOutEvent(QFocusEvent* event) {
    hasFocus_ = false;
    std::cout << "OctoFlexView lost focus" << std::endl;

    // Hide split/remove buttons when focus is lost.
    if (horizontalSplitButton_) horizontalSplitButton_->hide();
    if (verticalSplitButton_) verticalSplitButton_->hide();
    if (removeViewButton_) removeViewButton_->hide();
    if (expandButton_) expandButton_->hide();
    if (gridButton_) gridButton_->hide();

    // Clear all key states.
    keyW_ = false;
    keyA_ = false;
    keyS_ = false;
    keyD_ = false;
    keyQ_ = false;
    keyE_ = false;

    QOpenGLWidget::focusOutEvent(event);
}

// Event filter.
bool OctoFlexView::eventFilter(QObject* obj, QEvent* event) {
    // If keyboard event, ensure widget has focus.
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        if (!hasFocus_) {
            setFocus();
            std::cout << "OctoFlexView gained focus via event filter" << std::endl;
        }
    }

    // Check infoPanel_ validity and initialization.
    if (infoPanel_ && infoPanel_->getToggleButton() != nullptr &&
        (obj == infoPanel_->getToggleButton() || obj == horizontalSplitButton_ || obj == verticalSplitButton_ ||
         obj == removeViewButton_ || obj == expandButton_ || obj == gridButton_) &&
        (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
         event->type() == QEvent::MouseMove)) {
        // Ensure this view regains focus (prevent button events from stealing focus).
        if (!hasFocus_) {
            QTimer::singleShot(0, this, [this]() { this->setFocus(); });
        }

        return false;  // Let buttons handle mouse events.
    }

    // Continue event handling.
    return QOpenGLWidget::eventFilter(obj, event);
}

// Key press event.
void OctoFlexView::keyPressEvent(QKeyEvent* event) {
    qDebug() << "OctoFlexView::keyPressEvent - key pressed: " << event->key();

    // If widget has no focus, ignore key events.
    if (!hasFocus()) {
        qDebug() << "OctoFlexView::keyPressEvent - no focus, ignore key event";
        event->ignore();
        return;
    }

    // Debug log.
    std::cout << "OctoFlexView keyPressEvent: " << event->key() << std::endl;

    // Handle Esc key.
    if (event->key() == Qt::Key_Escape) {
        // Clear all selections.
        clearSelection();

        // Request repaint.
        update();

        // Stop event propagation.
        event->accept();
        return;
    }

    // Check for WASD or arrow keys.
    bool isWASD = false;
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_A:
        case Qt::Key_S:
        case Qt::Key_D:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            isWASD = true;
            break;
    }

    float deltaTime = 1.0f / 30.0f;  // Default value for first movement.

    // If WASD or arrows, check elapsed time.
    if (isWASD) {
        QTime currentTime = QTime::currentTime();
        if (!lastKeyPressTime_.isNull()) {
            int elapsed = lastKeyPressTime_.msecsTo(currentTime);
            deltaTime = elapsed / 1000.0f;  // Convert to seconds.
            if (elapsed > 100) {            // If interval exceeds 100 ms.
                currentSpeed_ = 0.0f;       // Reset speed.
            }
        }
        lastKeyPressTime_ = currentTime;
    }

    // Update movement state by key.
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            keyW_ = true;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            keyA_ = true;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            keyS_ = true;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            keyD_ = true;
            break;
        case Qt::Key_Q:
            keyQ_ = true;
            break;
        case Qt::Key_E:
            keyE_ = true;
            break;
        default:
            QOpenGLWidget::keyPressEvent(event);
            return;
    }

    // Compute movement direction.
    float deltaForward = 0.0f;
    float deltaRight = 0.0f;
    float deltaUp = 0.0f;

    if (keyW_) deltaForward += 1.0f;
    if (keyS_) deltaForward -= 1.0f;
    if (keyD_) deltaRight += 1.0f;
    if (keyA_) deltaRight -= 1.0f;
    if (keyE_) deltaUp += 1.0f;
    if (keyQ_) deltaUp -= 1.0f;

    // If any movement key is pressed, accelerate.
    if (deltaForward != 0.0f || deltaRight != 0.0f || deltaUp != 0.0f) {
        // Increase speed without exceeding max.
        currentSpeed_ = std::min(currentSpeed_ + acceleration_ * deltaTime, maxSpeed_);
    }

    // Handle camera movement.
    handleCameraMovement(deltaForward, deltaRight, deltaUp);

    // Update view matrix.
    viewMatrix_ = camera_->getViewMatrix();

    // Request repaint.
    update();

    // Stop event propagation.
    event->accept();
}

// Key release event.
void OctoFlexView::keyReleaseEvent(QKeyEvent* event) {
    // Debug log.
    std::cout << "OctoFlexView keyReleaseEvent: " << event->key() << std::endl;

    // Ignore auto-repeat events.
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    // Clear movement state by key.
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            keyW_ = false;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            keyA_ = false;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            keyS_ = false;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            keyD_ = false;
            break;
        case Qt::Key_Q:
            keyQ_ = false;
            break;
        case Qt::Key_E:
            keyE_ = false;
            break;
        default:
            QOpenGLWidget::keyReleaseEvent(event);
            return;
    }

    // Compute movement direction.
    float deltaForward = 0.0f;
    float deltaRight = 0.0f;
    float deltaUp = 0.0f;

    if (keyW_) deltaForward += 1.0f;
    if (keyS_) deltaForward -= 1.0f;
    if (keyD_) deltaRight += 1.0f;
    if (keyA_) deltaRight -= 1.0f;
    if (keyE_) deltaUp += 1.0f;
    if (keyQ_) deltaUp -= 1.0f;

    // If no movement keys are pressed, decelerate.
    if (deltaForward == 0.0f && deltaRight == 0.0f && deltaUp == 0.0f) {
        // Compute frame delta (assume 60 fps).
        float deltaTime = 1.0f / 60.0f;

        // Decrease speed but not below zero.
        currentSpeed_ = std::max(currentSpeed_ - deceleration_ * deltaTime, 0.0f);
    }

    // Handle camera movement.
    handleCameraMovement(deltaForward, deltaRight, deltaUp);

    // Update view matrix.
    viewMatrix_ = camera_->getViewMatrix();

    // Request repaint.
    update();

    // Stop event propagation.
    event->accept();
}

// Draw object info text.
void OctoFlexView::drawObjectInfoText() {
    // Create QPainter.
    QPainter painter(this);

    // Set text color and font.
    QFont font = painter.font();
    font.setBold(false);
    painter.setFont(font);

    // Draw all object info text.
    for (const auto& info : objectInfoToRender_) {
        painter.setPen(QColor(info.color.x * 255, info.color.y * 255, info.color.z * 255));
        // Draw text background (translucent rectangle).
        QFontMetrics fm = painter.fontMetrics();
        int textWidth = fm.horizontalAdvance(QString::fromStdString(info.text));
        int textHeight = fm.height();

        // Set background rect slightly larger than text.
        QRect bgRect(info.x - 2, info.y - textHeight, textWidth + 4, textHeight + 4);

        // Draw background.
        painter.fillRect(bgRect, QColor(255, 255, 255, 100));

        // Draw text.
        painter.drawText(info.x, info.y, QString::fromStdString(info.text));
    }
}

// Context menu event.
void OctoFlexView::contextMenuEvent(QContextMenuEvent* event) {
    // Create context menu.
    createContextMenu(event->pos());

    // Show menu.
    if (contextMenu_) {
        contextMenu_->exec(event->globalPos());
    }
}

void OctoFlexView::createContextMenu(const QPoint& pos) {
    // Delete old menu.
    if (contextMenu_) {
        delete contextMenu_;
        contextMenu_ = nullptr;
    }

    // Create new menu.
    contextMenu_ = new QMenu(this);

    // Add select objects menu item.
    QAction* selectObjectAction = new QAction("Select Objects...", this);
    connect(selectObjectAction, &QAction::triggered, [this]() { showObjectTreeDialog(false); });
    contextMenu_->addAction(selectObjectAction);

    // Add select layers menu item.
    QAction* selectLayerAction = new QAction("Select Layers...", this);
    connect(selectLayerAction, &QAction::triggered, [this]() { showObjectTreeDialog(true); });
    contextMenu_->addAction(selectLayerAction);

    // Add clear selection menu item.
    QAction* clearSelectionAction = new QAction("Clear Selection", this);
    connect(clearSelectionAction, &QAction::triggered, this, &OctoFlexView::clearSelection);
    contextMenu_->addAction(clearSelectionAction);

    // Add separator.
    contextMenu_->addSeparator();

    // Add selectable layers menu item.
    QAction* selectableLayerAction = new QAction("Selectable Layers...", this);
    connect(selectableLayerAction, &QAction::triggered, [this]() { showSelectableLayerDialog(); });
    contextMenu_->addAction(selectableLayerAction);

    // Add visible layers menu item.
    QAction* visibleLayerAction = new QAction("Visible Layers...", this);
    connect(visibleLayerAction, &QAction::triggered, [this]() { showVisibleLayerDialog(); });
    contextMenu_->addAction(visibleLayerAction);

    // Add separator.
    contextMenu_->addSeparator();

    // Add toggle projection menu item.
    QAction* toggleProjectionAction = new QAction(isPerspective_ ? "Orthographic" : "Perspective", this);
    connect(toggleProjectionAction, &QAction::triggered, this, &OctoFlexView::toggleProjection);
    contextMenu_->addAction(toggleProjectionAction);

    // Add separator.
    contextMenu_->addSeparator();

    // Add attach camera menu item.
    bool hasSelection = !selectedObjects_.empty();
    QString attachCameraText;
    if (hasSelection) {
        // Use first selected object for display
        QString firstObjId = QString::fromStdString(*selectedObjects_.begin());
        if (selectedObjects_.size() > 1) {
            attachCameraText = QString("Attach Camera to Object (%1+)").arg(firstObjId);
        } else {
            attachCameraText = QString("Attach Camera to Object (%1)").arg(firstObjId);
        }
    } else {
        attachCameraText = QString("Attach Camera to World");
    }
    QAction* attachCameraAction = new QAction(attachCameraText, this);
    connect(attachCameraAction, &QAction::triggered, [this]() {
        if (!selectedObjects_.empty()) {
            // Attach to first selected object
            std::string firstObjId = *selectedObjects_.begin();
            useObjectCoordinateSystem(firstObjId);
            std::cout << "Camera attached to object: " << firstObjId << std::endl;
        } else {
            // Attach to world (global coordinate system)
            useGlobalCoordinateSystem();
            std::cout << "Camera attached to world (global coordinate system)" << std::endl;
        }
        update();
    });
    contextMenu_->addAction(attachCameraAction);

    // Add separator.
    contextMenu_->addSeparator();

    // Add camera view menu with sub-items.
    QMenu* cameraViewMenu = contextMenu_->addMenu("Camera View");

    // Determine menu text based on selection.
    QString targetText = hasSelection ? " (to Selected Object)" : " (to Origin)";
    QString viewTarget = hasSelection ? "object" : "origin";

    QAction* frontViewAction = new QAction("Front View" + targetText, this);
    connect(frontViewAction, &QAction::triggered, [this]() { setCameraView("front"); });
    cameraViewMenu->addAction(frontViewAction);

    QAction* backViewAction = new QAction("Back View" + targetText, this);
    connect(backViewAction, &QAction::triggered, [this]() { setCameraView("back"); });
    cameraViewMenu->addAction(backViewAction);

    QAction* topViewAction = new QAction("Top View" + targetText, this);
    connect(topViewAction, &QAction::triggered, [this]() { setCameraView("top"); });
    cameraViewMenu->addAction(topViewAction);

    QAction* bottomViewAction = new QAction("Bottom View" + targetText, this);
    connect(bottomViewAction, &QAction::triggered, [this]() { setCameraView("bottom"); });
    cameraViewMenu->addAction(bottomViewAction);

    QAction* leftViewAction = new QAction("Left View" + targetText, this);
    connect(leftViewAction, &QAction::triggered, [this]() { setCameraView("left"); });
    cameraViewMenu->addAction(leftViewAction);

    QAction* rightViewAction = new QAction("Right View" + targetText, this);
    connect(rightViewAction, &QAction::triggered, [this]() { setCameraView("right"); });
    cameraViewMenu->addAction(rightViewAction);
}

void OctoFlexView::showObjectTreeDialog(bool layerOnly) {
    // Debug log.
    std::cout << "Show object tree dialog" << std::endl;

    // Check object manager.
    if (!obj_mgr_) {
        std::cout << "Error: object manager is null" << std::endl;
        return;
    }

    // Log layers and objects in object manager.
    const auto& layers = obj_mgr_->layers();
    std::cout << "Layer count in object manager: " << layers.size() << std::endl;

    for (const auto& layerPair : layers) {
        std::cout << "Layer [" << layerPair.first << "] object count: " << layerPair.second->objects().size()
                  << std::endl;
    }

    // Create dialog.
    ObjectTreeDialog dialog(this);

    // Set dialog mode.
    if (layerOnly) {
        dialog.setMode(ObjectTreeMode::LAYER_ONLY);
    } else {
        dialog.setMode(ObjectTreeMode::ALL);
    }

    // Set object manager.
    dialog.setObjectManager(obj_mgr_);

    // Show dialog.
    if (dialog.exec() == QDialog::Accepted) {
        // Get selected objects.
        std::set<std::string> selectedObjects = dialog.getSelectedObjects();

        // Log selected objects.
        std::cout << "Selected object count: " << selectedObjects.size() << std::endl;
        for (const auto& objId : selectedObjects) {
            std::cout << "  - " << objId << std::endl;
        }

        // Clear current selection.
        clearSelection();

        // Select new objects.
        for (const std::string& objId : selectedObjects) {
            selectObject(objId, true);
        }

        // Update object list.
        updateObjectList();

        // Trigger repaint.
        update();
    } else {
        std::cout << "Dialog canceled" << std::endl;
    }
}

void OctoFlexView::showSelectableLayerDialog() {
    // Debug log.
    std::cout << "Show selectable layers dialog" << std::endl;

    // Check object manager.
    if (!obj_mgr_) {
        std::cout << "Error: object manager is null" << std::endl;
        return;
    }

    // Create dialog.
    ObjectTreeDialog dialog(this);
    dialog.setWindowTitle("Select Selectable Layers");

    // Set dialog mode to layers only.
    dialog.setMode(ObjectTreeMode::LAYER_ONLY);

    // Set object manager.
    dialog.setObjectManager(obj_mgr_);

    // Prepare preselected layers (all selectable layers).
    std::set<std::string> selectableLayers;
    for (const auto& [layerId, layer] : obj_mgr_->layers()) {
        // Add if not in unselectable set.
        if (unselectable_layers_.find(layerId) == unselectable_layers_.end()) {
            selectableLayers.insert(layerId);
        }
    }

    // Set preselected items.
    dialog.setPreselectedItems(selectableLayers);

    // Show dialog.
    if (dialog.exec() == QDialog::Accepted) {
        // Get selected layers.
        std::set<std::string> selectedLayers = dialog.getSelectedObjects();

        // Log selected layers.
        std::cout << "Selected layer count: " << selectedLayers.size() << std::endl;
        for (const auto& layerId : selectedLayers) {
            std::cout << "  - " << layerId << std::endl;
        }

        // Clear unselectable layers.
        unselectable_layers_.clear();

        // Add unselected layers to unselectable set.
        for (const auto& [layerId, layer] : obj_mgr_->layers()) {
            // Add if not in selected set.
            if (selectedLayers.find(layerId) == selectedLayers.end()) {
                unselectable_layers_.insert(layerId);
                std::cout << "Added to unselectable layers: " << layerId << std::endl;
            }
        }

        // Trigger repaint.
        update();
    } else {
        std::cout << "Dialog canceled" << std::endl;
    }
}

void OctoFlexView::showVisibleLayerDialog() {
    // Debug log.
    std::cout << "Show visible layers dialog" << std::endl;

    // Check object manager.
    if (!obj_mgr_) {
        std::cout << "Error: object manager is null" << std::endl;
        return;
    }

    // Create dialog.
    ObjectTreeDialog dialog(this);
    dialog.setWindowTitle("Select Visible Layers");

    // Set dialog mode to layers only.
    dialog.setMode(ObjectTreeMode::LAYER_ONLY);

    // Set object manager.
    dialog.setObjectManager(obj_mgr_);

    // Prepare preselected layers (all visible layers).
    std::set<std::string> visibleLayers;
    for (const auto& [layerId, layer] : obj_mgr_->layers()) {
        // Add if not in hidden set.
        if (unvisable_layers_.find(layerId) == unvisable_layers_.end()) {
            visibleLayers.insert(layerId);
        }
    }

    // Set preselected items.
    dialog.setPreselectedItems(visibleLayers);

    // Show dialog.
    if (dialog.exec() == QDialog::Accepted) {
        // Get selected layers.
        std::set<std::string> selectedLayers = dialog.getSelectedObjects();

        // Log selected layers.
        std::cout << "Selected layer count: " << selectedLayers.size() << std::endl;
        for (const auto& layerId : selectedLayers) {
            std::cout << "  - " << layerId << std::endl;
        }

        // Clear hidden layers.
        unvisable_layers_.clear();

        // Add unselected layers to hidden set.
        for (const auto& [layerId, layer] : obj_mgr_->layers()) {
            // Add if not in selected set.
            if (selectedLayers.find(layerId) == selectedLayers.end()) {
                unvisable_layers_.insert(layerId);
                std::cout << "Added to hidden layers: " << layerId << std::endl;
            }
        }

        // Trigger repaint.
        update();
    } else {
        std::cout << "Dialog canceled" << std::endl;
    }
}

// Draw info panel.
void OctoFlexView::drawInfoPanel() {
    QPainter painter(this);
    infoPanel_->draw(painter);
}

// Info panel helpers.
void OctoFlexView::setInfoItem(const std::string& id, const std::string& info) {
    if (infoPanel_) {
        infoPanel_->setInfoItem(id, info, InfoItemType::NORMAL);
    }
}

void OctoFlexView::setWarningItem(const std::string& id, const std::string& info) {
    if (infoPanel_) {
        infoPanel_->setInfoItem(id, info, InfoItemType::WARNING);
    }
}

void OctoFlexView::setErrorItem(const std::string& id, const std::string& info) {
    if (infoPanel_) {
        infoPanel_->setInfoItem(id, info, InfoItemType::ERROR);
    }
}

void OctoFlexView::removeInfoItem(const std::string& id) {
    if (infoPanel_) {
        infoPanel_->removeInfoItem(id);
    }
}

void OctoFlexView::clearInfoItems() {
    if (infoPanel_) {
        infoPanel_->clearInfoItems();
    }
}

void OctoFlexView::toggleInfoPanel() {
    if (infoPanel_) {
        // Log current state before toggling.
        std::cout << "OctoFlexView::toggleInfoPanel: current state - " << (infoPanel_->isVisible() ? "shown" : "hidden")
                  << std::endl;

        // Toggle state.
        infoPanel_->toggle();

        // Log state after toggling.
        std::cout << "OctoFlexView::toggleInfoPanel: new state - " << (infoPanel_->isVisible() ? "shown" : "hidden")
                  << std::endl;

        // Ensure view updates.
        update();
    }
}

bool OctoFlexView::isInfoPanelVisible() const { return infoPanel_ ? infoPanel_->isVisible() : false; }

InfoPanel* OctoFlexView::getInfoPanel() const { return infoPanel_; }

const std::set<std::string>& OctoFlexView::getUnvisableLayers() const { return unvisable_layers_; }

void OctoFlexView::setUnvisableLayers(const std::set<std::string>& layers) {
    unvisable_layers_ = layers;
    update();  // Refresh view.
}

const std::set<std::string>& OctoFlexView::getUnselectableLayers() const { return unselectable_layers_; }

void OctoFlexView::setUnselectableLayers(const std::set<std::string>& layers) { unselectable_layers_ = layers; }

bool OctoFlexView::isPerspectiveMode() const { return isPerspective_; }

void OctoFlexView::setPerspectiveMode(bool isPerspective) {
    if (isPerspective_ != isPerspective) {
        isPerspective_ = isPerspective;

        // Update projection matrix.
        if (isPerspective_) {
            projectionMatrix_ = perspectiveMatrix_;
        } else {
            projectionMatrix_ = orthoMatrix_;
        }

        update();  // Refresh view.
    }
}

// Ensure the view regains focus.
void OctoFlexView::ensureFocus() {
    // Use a delayed call to regain focus after button events.
    QTimer::singleShot(0, this, [this]() {
        if (!hasFocus_) {
            setFocus();
            std::cout << "OctoFlexView ensureFocus: view regained focus" << std::endl;
        }
    });
}

void OctoFlexView::updateFpsInfo() {
    // Compute current FPS.
    currentFps_ = frameCount_ / 1.0f;
    frameCount_ = 0;

    // Format FPS with one decimal.
    char fpsStr[32];
    snprintf(fpsStr, sizeof(fpsStr), "%.1f", currentFps_);
    std::string fpsText = "FPS: " + std::string(fpsStr);

    // Update FPS info.
    setInfoItem("fps", fpsText);
}

// Set view ID.
void OctoFlexView::setViewId(const std::string& id) {
    viewId_ = id;
    setInfoItem("view_id", "View ID: " + id);
    update();
}

// Get view ID.
std::string OctoFlexView::getViewId() const { return viewId_; }

// Capture current frame from OpenGL framebuffer.
QImage OctoFlexView::captureFrame() {
    // Read pixels from the OpenGL framebuffer
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    if (width <= 0 || height <= 0) {
        return QImage();
    }

    // Allocate buffer for RGBA pixels
    std::vector<GLubyte> pixels(width * height * 4);

    // Read pixels from the framebuffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL reads from bottom-left, QImage expects top-left
    // Flip the image vertically
    QImage image(width, height, QImage::Format_RGBA8888);
    for (int y = 0; y < height; ++y) {
        const GLubyte* src = pixels.data() + (height - 1 - y) * width * 4;
        memcpy(image.scanLine(y), src, width * 4);
    }

    return image;
}

void OctoFlexView::expandView() {
    if (!isExpanded_) {
        isExpanded_ = true;
        expandButton_->setText("◱");  // Switch to collapse icon.
        expandButton_->setToolTip("Collapse View");
        setInfoItem("view_state", "State: Expanded");

        // Update button states.
        updateButtonStates();
    }
}

void OctoFlexView::collapseView() {
    if (isExpanded_) {
        isExpanded_ = false;
        expandButton_->setText("⛶");  // Switch to expand icon.
        expandButton_->setToolTip("Expand View");
        removeInfoItem("view_state");

        // Update button states.
        updateButtonStates();
    }
}

bool OctoFlexView::isExpanded() const { return isExpanded_; }

bool OctoFlexView::isGridVisible() const { return showGrid_; }

void OctoFlexView::setIsOnlyView(bool isOnly) {
    if (isOnlyView_ != isOnly) {
        isOnlyView_ = isOnly;

        // Update button states.
        updateButtonStates();
    }
}

void OctoFlexView::updateButtonStates() {
    // Disable expand button when only one view exists.
    if (expandButton_) {
        expandButton_->setEnabled(!isOnlyView_);
    }

    // Disable split/remove buttons when view is expanded.
    if (horizontalSplitButton_) {
        horizontalSplitButton_->setEnabled(!isExpanded_);
    }

    if (verticalSplitButton_) {
        verticalSplitButton_->setEnabled(!isExpanded_);
    }

    if (removeViewButton_) {
        removeViewButton_->setEnabled(!isExpanded_);
    }
}

void OctoFlexView::toggleGrid() {
    // Toggle grid visibility.
    showGrid_ = !showGrid_;

    // Update button text.
    if (gridButton_) {
        if (showGrid_) {
            gridButton_->setText("□");  // Use "minus grid" icon when shown.
            gridButton_->setToolTip("Hide Grid");
            setInfoItem("grid_state", "Grid: Shown");
        } else {
            gridButton_->setText("⊞");  // Use "plus grid" icon when hidden.
            gridButton_->setToolTip("Show Grid");
            removeInfoItem("grid_state");
        }
    }

    // Request repaint.
    update();
}

void OctoFlexView::renderGrid() {
    // Get current camera position.
    glm::vec3 cameraPos = camera_->getPosition();

    // Grid parameters.
    const float gridSize = 10.0f;  // Grid cell size (10 meters).
    const float gridExtent = 50;   // Grid spans 10 cells per side (100 meters).

    // Compute grid origin centered on camera.
    float startX = cameraPos.x - gridExtent * gridSize / 2;
    float startY = cameraPos.y - gridExtent * gridSize / 2;

    // Snap origin to grid lines.
    startX = floor(startX / gridSize) * gridSize;
    startY = floor(startY / gridSize) * gridSize;

    // Begin drawing grid lines.
    glBegin(GL_LINES);

    // Color by distance (near = gray, far = white).
    float maxDist = gridExtent * gridSize / 2;
    float z = -1e-3;
    // Draw grid lines on XY plane.
    for (int i = 0; i <= gridExtent; ++i) {
        float y = startY + i * gridSize;

        for (int j = 0; j <= gridExtent; ++j) {
            float x = startX + j * gridSize;

            // Compute XY-plane distance to camera.
            float dx = x - cameraPos.x;
            float dy = y - cameraPos.y;
            float distFromCamera = sqrt(dx * dx + dy * dy);

            float t = std::min(distFromCamera / maxDist, 1.0f);
            float gray = 0.5f + t * 0.5f;  // From ~0.39 to 1.0 (white).

            // Set color (opaque).
            glColor3f(gray, gray, gray);

            glVertex3f(x, y, z);
            glVertex3f(x + gridSize, y, z);

            glVertex3f(x, y, z);
            glVertex3f(x, y + gridSize, z);
        }
    }

    glEnd();
}

void OctoFlexView::renderCoordinateSystem() {
    // Axis parameters.
    const float axisLength = 5.0f;   // Axis length.
    const float arrowHeight = 0.3f;  // Arrow height (along axis).
    const float arrowWidth = 0.08f;  // Arrow width (perpendicular to axis).

    // Draw axes.
    glBegin(GL_LINES);

    // X axis - red.
    glColor3f(1.0f, 0.0f, 0.0f);         // Red.
    glVertex3f(0.0f, 0.0f, 0.0f);        // Origin.
    glVertex3f(axisLength, 0.0f, 0.0f);  // +X.

    // Y axis - green.
    glColor3f(0.0f, 1.0f, 0.0f);         // Green.
    glVertex3f(0.0f, 0.0f, 0.0f);        // Origin.
    glVertex3f(0.0f, axisLength, 0.0f);  // +Y.

    // Z axis - blue.
    glColor3f(0.0f, 0.0f, 1.0f);         // Blue.
    glVertex3f(0.0f, 0.0f, 0.0f);        // Origin.
    glVertex3f(0.0f, 0.0f, axisLength);  // +Z.

    glEnd();

    // Draw X-axis arrow.
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);                              // Red.
    glVertex3f(axisLength, 0.0f, 0.0f);                       // Tip.
    glVertex3f(axisLength - arrowHeight, arrowWidth, 0.0f);   // Base.
    glVertex3f(axisLength - arrowHeight, -arrowWidth, 0.0f);  // Base.
    glEnd();

    // Draw Y-axis arrow.
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 1.0f, 0.0f);                              // Green.
    glVertex3f(0.0f, axisLength, 0.0f);                       // Tip.
    glVertex3f(arrowWidth, axisLength - arrowHeight, 0.0f);   // Base.
    glVertex3f(-arrowWidth, axisLength - arrowHeight, 0.0f);  // Base.
    glEnd();

    // Draw Z-axis arrow.
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.0f, 1.0f);                              // Blue.
    glVertex3f(0.0f, 0.0f, axisLength);                       // Tip.
    glVertex3f(arrowWidth, 0.0f, axisLength - arrowHeight);   // Base.
    glVertex3f(-arrowWidth, 0.0f, axisLength - arrowHeight);  // Base.
    glEnd();
}

void OctoFlexView::handleCameraMovement(float deltaForward, float deltaRight, float deltaUp) {
    // If there is forward/back movement, try using mouse ray direction.
    if (deltaForward != 0.0f) {
        // Get current mouse position.
        QPoint mousePos = mapFromGlobal(QCursor::pos());

        // Check if mouse is inside the window.
        if (rect().contains(mousePos)) {
            // Convert mouse position to normalized device coordinates (-1 to 1).
            float x = (2.0f * mousePos.x()) / width() - 1.0f;
            float y = 1.0f - (2.0f * mousePos.y()) / height();

            // Create point on near plane.
            glm::vec4 rayClip(x, y, -1.0f, 1.0f);

            // Transform to view space.
            glm::mat4 invProj = glm::inverse(projectionMatrix_);
            glm::vec4 rayEye = invProj * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

            // Transform to world space.
            glm::mat4 invView = glm::inverse(viewMatrix_);
            glm::vec4 rayWorld = invView * rayEye;
            glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

            // Move along ray direction using current speed.
            camera_->moveWithRay(deltaForward * currentSpeed_, deltaRight * currentSpeed_, deltaUp * currentSpeed_,
                                 rayDirection);
        } else {
            // If mouse is outside, use default forward direction.
            camera_->move(deltaForward * currentSpeed_, deltaRight * currentSpeed_, deltaUp * currentSpeed_);
        }
    } else {
        // No forward/back movement; use standard movement.
        camera_->move(0.0f, deltaRight * currentSpeed_, deltaUp * currentSpeed_);
    }
}

// ============================================================================
// Coordinate System Management
// ============================================================================

void OctoFlexView::setCoordinateSystem(CoordinateSystem::Ptr coordSys) {
    if (camera_) {
        camera_->setCoordinateSystem(coordSys);
    }
}

CoordinateSystem::Ptr OctoFlexView::getCoordinateSystem() const {
    if (camera_) {
        return camera_->getCoordinateSystem();
    }
    return nullptr;
}

void OctoFlexView::useGlobalCoordinateSystem() {
    setCoordinateSystem(CoordinateSystem::createGlobal());

    // Disable roll and reset camera up to world up when attached to world
    if (camera_) {
        camera_->setRollEnabled(false);

        // Reset camera roll: align up with world Z axis
        glm::vec3 front = camera_->getFront();
        glm::vec3 worldUp(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        camera_->setVectors(front, up, right);

        std::cout << "Camera roll disabled and reset (attached to world)" << std::endl;
    }
}

void OctoFlexView::useObjectCoordinateSystem(const std::string& objectId) {
    if (!obj_mgr_) {
        return;
    }

    auto [layerId, obj] = obj_mgr_->findObject(objectId);
    if (!obj) {
        return;
    }

    // Get object position and orientation
    Vec3 objPos = obj->position();
    glm::vec3 position(objPos.x, objPos.y, objPos.z);
    Quaternion objOrientation = obj->orientation();

    // Convert to GLM quaternion
    glm::quat orientation(objOrientation.w, objOrientation.x, objOrientation.y, objOrientation.z);

    // Create local coordinate system from object transform
    auto coordSys = CoordinateSystem::createLocal(position, orientation, objectId);
    setCoordinateSystem(coordSys);

    // Enable roll when attached to object
    if (camera_) {
        camera_->setRollEnabled(true);
        std::cout << "Camera roll enabled (attached to object)" << std::endl;
    }
}

CoordinateSystemType OctoFlexView::getCoordinateSystemType() const {
    auto coordSys = getCoordinateSystem();
    if (coordSys) {
        return coordSys->getType();
    }
    return CoordinateSystemType::Global;
}

void OctoFlexView::updateCoordinateSystem() {
    auto coordSys = getCoordinateSystem();
    if (!coordSys || coordSys->getType() != CoordinateSystemType::Local) {
        return;  // No coordinate system or global coordinate system - nothing to update
    }

    const std::string& objectId = coordSys->getObjectId();
    if (objectId.empty()) {
        return;  // No associated object ID
    }

    if (!obj_mgr_) {
        return;
    }

    auto [layerId, obj] = obj_mgr_->findObject(objectId);
    if (!obj) {
        // Object no longer exists - invalidate or switch to global
        std::cout << "Attached object '" << objectId << "' no longer exists, switching to global coordinate system"
                  << std::endl;
        useGlobalCoordinateSystem();
        return;
    }

    // IMPORTANT: Store camera's relative transform in OLD coordinate system BEFORE updating coord sys
    glm::vec3 oldLocalPosition = coordSys->worldToLocal(camera_->getPosition());
    glm::vec3 oldLocalFront = coordSys->worldDirectionToLocal(camera_->getFront());
    glm::vec3 oldLocalUp = coordSys->worldDirectionToLocal(camera_->getUp());

    // Get current object position and orientation
    Vec3 objPos = obj->position();
    glm::vec3 position(objPos.x, objPos.y, objPos.z);
    Quaternion objOrientation = obj->orientation();

    // Convert to GLM quaternion
    glm::quat orientation(objOrientation.w, objOrientation.x, objOrientation.y, objOrientation.z);

    // NOW update coordinate system with new transform
    coordSys->setPosition(position);
    coordSys->setOrientation(orientation);

    // Transform camera's stored relative position/direction to the NEW coordinate system
    glm::vec3 newWorldPosition = coordSys->localToWorld(oldLocalPosition);
    glm::vec3 newWorldFront = coordSys->localDirectionToWorld(oldLocalFront);
    glm::vec3 newWorldUp = coordSys->localDirectionToWorld(oldLocalUp);

    // Recalculate right vector to ensure orthogonality
    glm::vec3 newWorldRight = glm::normalize(glm::cross(newWorldFront, newWorldUp));
    // Recalculate up to ensure orthogonality
    newWorldUp = glm::normalize(glm::cross(newWorldRight, newWorldFront));

    // Update camera position and orientation
    camera_->setPosition(newWorldPosition);
    camera_->setVectors(newWorldFront, newWorldUp, newWorldRight);
}

}  // namespace octo_flex
