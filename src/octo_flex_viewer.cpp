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

#include "octo_flex_viewer.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <iostream>
#include <vector>

#include "def.h"
#include "object.h"
#include "object_manager.h"
#include "octo_flex_view.h"
#include "octo_flex_view_container.h"
#include "utils.h"

namespace octo_flex {

// ============================================================================
// EmbeddedViewer::Impl - Private implementation for embedded mode
// ============================================================================
struct EmbeddedViewer::Impl {
    OctoFlexViewContainer* container = nullptr;  // View container
    ObjectManager::Ptr obj_manager;              // Object manager

    ~Impl() {
        // Note: container is deleted by Qt parent-child mechanism
        // Only delete if it has no parent
        if (container && !container->parent()) {
            delete container;
            container = nullptr;
        }
    }
};

// ============================================================================
// EmbeddedViewer - Public interface implementation
// ============================================================================

EmbeddedViewer::EmbeddedViewer() : impl_(std::make_unique<Impl>()) {}

EmbeddedViewer::~EmbeddedViewer() = default;

EmbeddedViewer::EmbeddedViewer(EmbeddedViewer&&) noexcept = default;

EmbeddedViewer& EmbeddedViewer::operator=(EmbeddedViewer&&) noexcept = default;

EmbeddedViewer& EmbeddedViewer::add(std::shared_ptr<Object> object) { return add(object, "default"); }

EmbeddedViewer& EmbeddedViewer::add(std::shared_ptr<Object> object, const std::string& layer_id) {
    if (!impl_->obj_manager) {
        std::cerr << "Error: Object manager not initialized" << std::endl;
        return *this;
    }

    impl_->obj_manager->submit(object, layer_id);
    return *this;
}

EmbeddedViewer& EmbeddedViewer::setLayer(const std::vector<std::shared_ptr<Object>>& objects,
                                         const std::string& layer_id) {
    if (!impl_->obj_manager) {
        std::cerr << "Error: Object manager not initialized" << std::endl;
        return *this;
    }

    impl_->obj_manager->submitLayer(objects, layer_id);
    return *this;
}

EmbeddedViewer& EmbeddedViewer::addSphere(const std::string& id, const Vec3& color, double radius,
                                          const Vec3& position) {
    auto sphere = generateSphere(id, color, radius, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        sphere->move(position);
    }
    return add(sphere);
}

EmbeddedViewer& EmbeddedViewer::addBox(const std::string& id, const Vec3& color, double width, double height,
                                       double depth, const Vec3& position) {
    auto box = generateCubic(id, color, width, height, depth, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        box->move(position);
    }
    return add(box);
}

EmbeddedViewer& EmbeddedViewer::addCylinder(const std::string& id, const Vec3& color, double radius, double height,
                                            const Vec3& position) {
    auto cylinder = generateCylinder(id, color, radius, height, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        cylinder->move(position);
    }
    return add(cylinder);
}

QWidget* EmbeddedViewer::widget() const { return impl_->container; }

std::vector<std::string> EmbeddedViewer::exportSelectedObjectIds() const {
    if (!impl_->container) {
        return {};
    }

    OctoFlexView* currentView = impl_->container->getCurrentView();
    if (!currentView) {
        return {};
    }

    const std::set<std::string>& selectedIds = currentView->getSelectedObjects();
    return std::vector<std::string>(selectedIds.begin(), selectedIds.end());
}

bool EmbeddedViewer::startRecording(const RecordingOptions& options) {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->startRecording(options);
}

bool EmbeddedViewer::pauseRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->pauseRecording();
}

bool EmbeddedViewer::resumeRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->resumeRecording();
}

bool EmbeddedViewer::stopRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->stopRecording();
}

bool EmbeddedViewer::isRecording() const { return impl_->container && impl_->container->isRecording(); }

bool EmbeddedViewer::isRecordingPaused() const {
    return impl_->container && impl_->container->isRecordingPaused();
}

std::string EmbeddedViewer::getLastRecordingError() const {
    if (!impl_->container) {
        return "Container not initialized";
    }
    return impl_->container->getLastRecordingError();
}

// ============================================================================
// OctoFlexViewer::Impl - Private implementation
// ============================================================================
struct OctoFlexViewer::Impl {
    std::unique_ptr<QApplication> app;               // Owned Qt application (if created)
    bool owns_app = false;                           // Whether we own the QApplication
    QMainWindow* window = nullptr;                   // Main window
    OctoFlexViewContainer* container = nullptr;      // View container
    ObjectManager::Ptr obj_manager;                  // Object manager
    std::vector<std::function<void()>> pending_ops;  // Pending operations
    bool context_ready = false;                      // OpenGL context ready flag

    // Dummy argc/argv for Qt when user doesn't provide them
    int dummy_argc = 1;
    std::vector<char> dummy_argv_data{'a', 'p', 'p', '\0'};
    char* dummy_argv_ptr;
    char** dummy_argv;

    Impl() {
        // Setup dummy command line args
        dummy_argv_ptr = dummy_argv_data.data();
        dummy_argv = &dummy_argv_ptr;
    }

    ~Impl() {
        // Clean up in reverse order of creation
        if (window) {
            delete window;
            window = nullptr;
        }
        // Note: container is deleted by window (parent-child relationship)
        // Note: app is automatically deleted by unique_ptr
    }

    /**
     * @brief Ensure OpenGL context is ready
     * Executes all pending operations after context becomes available
     */
    void ensureContextReady() {
        if (context_ready) {
            return;
        }

        // Show window to initialize OpenGL context
        if (window && !window->isVisible()) {
            window->show();
            QCoreApplication::processEvents();  // Process pending events
        }

        context_ready = true;

        // Execute all pending operations
        for (auto& op : pending_ops) {
            op();
        }
        pending_ops.clear();
    }
};

// ============================================================================
// OctoFlexViewer - Public interface implementation
// ============================================================================

OctoFlexViewer::OctoFlexViewer() : impl_(std::make_unique<Impl>()) {}

OctoFlexViewer::~OctoFlexViewer() = default;

OctoFlexViewer::OctoFlexViewer(OctoFlexViewer&&) noexcept = default;

OctoFlexViewer& OctoFlexViewer::operator=(OctoFlexViewer&&) noexcept = default;

EmbeddedViewer OctoFlexViewer::createEmbedded(QWidget* parent) {
    EmbeddedViewer viewer;

    // Step 1: Check if QApplication exists
    if (!qApp) {
        std::cerr << "Error: QApplication must be created before calling createEmbedded()" << std::endl;
        std::cerr << "Please create QApplication in your main() function first." << std::endl;
        return viewer;
    }

    // Step 2: Create object manager
    viewer.impl_->obj_manager = std::make_shared<ObjectManager>();

    // Step 3: Create container widget
    viewer.impl_->container = new OctoFlexViewContainer(parent);

    // Step 4: Connect container to object manager
    viewer.impl_->container->setObjectManager(viewer.impl_->obj_manager);

    // Step 5: Create initial view
    viewer.impl_->container->createInitialView();

    return viewer;
}

OctoFlexViewer OctoFlexViewer::create(const std::string& title, int width, int height, int* argc, char*** argv) {
    OctoFlexViewer viewer;

    // Step 1: Create or reuse QApplication
    if (!qApp) {
        // No existing QApplication, create one
        if (argc && argv) {
            viewer.impl_->app = std::make_unique<QApplication>(*argc, *argv);
        } else {
            // Use dummy argc/argv
            viewer.impl_->app = std::make_unique<QApplication>(viewer.impl_->dummy_argc, viewer.impl_->dummy_argv);
        }
        viewer.impl_->owns_app = true;
    } else {
        // Reuse existing QApplication
        viewer.impl_->owns_app = false;
    }

    // Step 2: Create object manager
    viewer.impl_->obj_manager = std::make_shared<ObjectManager>();

    // Step 3: Create main window
    viewer.impl_->window = new QMainWindow();
    viewer.impl_->window->setWindowTitle(QString::fromStdString(title));
    viewer.impl_->window->resize(width, height);

    // Step 4: Create container and set as central widget
    viewer.impl_->container = new OctoFlexViewContainer(viewer.impl_->window);
    viewer.impl_->window->setCentralWidget(viewer.impl_->container);

    // Step 5: Connect container to object manager
    viewer.impl_->container->setObjectManager(viewer.impl_->obj_manager);

    // Step 6: Create initial view
    viewer.impl_->container->createInitialView();

    return viewer;
}

int OctoFlexViewer::run(SetupCallback setup) {
    if (!impl_->window) {
        std::cerr << "Error: OctoFlexViewer not properly initialized" << std::endl;
        return -1;
    }

    // Execute setup callback if provided
    if (setup) {
        impl_->ensureContextReady();
        setup(*this);
    }

    // Show window if not already visible
    if (!impl_->window->isVisible()) {
        impl_->window->show();
    }

    // Run event loop if we own the QApplication
    if (impl_->owns_app && impl_->app) {
        return impl_->app->exec();
    } else if (qApp) {
        // Use existing QApplication
        return qApp->exec();
    } else {
        std::cerr << "Error: No QApplication available" << std::endl;
        return -1;
    }
}

OctoFlexViewer& OctoFlexViewer::add(std::shared_ptr<Object> object) { return add(object, "default"); }

OctoFlexViewer& OctoFlexViewer::add(std::shared_ptr<Object> object, const std::string& layer_id) {
    if (!impl_->obj_manager) {
        std::cerr << "Error: Object manager not initialized" << std::endl;
        return *this;
    }

    impl_->obj_manager->submit(object, layer_id);
    return *this;
}

OctoFlexViewer& OctoFlexViewer::setLayer(const std::vector<std::shared_ptr<Object>>& objects,
                                         const std::string& layer_id) {
    if (!impl_->obj_manager) {
        std::cerr << "Error: Object manager not initialized" << std::endl;
        return *this;
    }

    impl_->obj_manager->submitLayer(objects, layer_id);
    return *this;
}

OctoFlexViewer& OctoFlexViewer::addSphere(const std::string& id, const Vec3& color, double radius,
                                          const Vec3& position) {
    auto sphere = generateSphere(id, color, radius, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        sphere->move(position);
    }
    return add(sphere);
}

OctoFlexViewer& OctoFlexViewer::addBox(const std::string& id, const Vec3& color, double width, double height,
                                       double depth, const Vec3& position) {
    auto box = generateCubic(id, color, width, height, depth, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        box->move(position);
    }
    return add(box);
}

OctoFlexViewer& OctoFlexViewer::addCylinder(const std::string& id, const Vec3& color, double radius, double height,
                                            const Vec3& position) {
    auto cylinder = generateCylinder(id, color, radius, height, true);
    if (position.x != 0.0 || position.y != 0.0 || position.z != 0.0) {
        cylinder->move(position);
    }
    return add(cylinder);
}

std::shared_ptr<ObjectManager> OctoFlexViewer::objectManager() const { return impl_->obj_manager; }

QWidget* OctoFlexViewer::container() const { return impl_->container; }

void OctoFlexViewer::show() {
    if (impl_->window) {
        impl_->window->show();
        impl_->ensureContextReady();
    }
}

void OctoFlexViewer::setTitle(const std::string& title) {
    if (impl_->window) {
        impl_->window->setWindowTitle(QString::fromStdString(title));
    }
}

void OctoFlexViewer::resize(int width, int height) {
    if (impl_->window) {
        impl_->window->resize(width, height);
    }
}

std::vector<std::string> OctoFlexViewer::exportSelectedObjectIds() const {
    if (!impl_->container) {
        return {};
    }

    OctoFlexView* currentView = impl_->container->getCurrentView();
    if (!currentView) {
        return {};
    }

    const std::set<std::string>& selectedIds = currentView->getSelectedObjects();
    return std::vector<std::string>(selectedIds.begin(), selectedIds.end());
}

bool OctoFlexViewer::startRecording(const RecordingOptions& options) {
    if (!impl_->container) {
        return false;
    }

    if (impl_->window && !impl_->window->isVisible()) {
        impl_->window->show();
        QCoreApplication::processEvents();
    }
    return impl_->container->startRecording(options);
}

bool OctoFlexViewer::pauseRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->pauseRecording();
}

bool OctoFlexViewer::resumeRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->resumeRecording();
}

bool OctoFlexViewer::stopRecording() {
    if (!impl_->container) {
        return false;
    }
    return impl_->container->stopRecording();
}

bool OctoFlexViewer::isRecording() const { return impl_->container && impl_->container->isRecording(); }

bool OctoFlexViewer::isRecordingPaused() const {
    return impl_->container && impl_->container->isRecordingPaused();
}

std::string OctoFlexViewer::getLastRecordingError() const {
    if (!impl_->container) {
        return "Container not initialized";
    }
    return impl_->container->getLastRecordingError();
}

}  // namespace octo_flex
