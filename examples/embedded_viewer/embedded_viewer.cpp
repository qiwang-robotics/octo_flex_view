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

/**
 * @file embedded_viewer.cpp
 * @brief Simple example: Embed OctoFlexView in a Qt window
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <cstdlib>
#include <cmath>

#include "octo_flex_viewer.h"

using namespace octo_flex;

// Application state
struct AppState {
    EmbeddedViewer* viewer = nullptr;
    int objectCounter = 0;
};

// Generate random position within [-2, 2] meters box
Vec3 randomPosition() {
    constexpr double RANGE = 2.0;
    double x = (rand() / (double)RAND_MAX) * 2 * RANGE - RANGE;
    double y = (rand() / (double)RAND_MAX) * 2 * RANGE - RANGE;
    double z = (rand() / (double)RAND_MAX) * 2 * RANGE - RANGE;
    return Vec3(x, y, z);
}

// Create left button panel
QWidget* createButtonPanel(AppState* state) {
    QWidget* panel = new QWidget();
    panel->setFixedWidth(120);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    QPushButton* btnSphere = new QPushButton("Add Sphere");
    QPushButton* btnBox = new QPushButton("Add Box");
    QPushButton* btnClear = new QPushButton("Clear All");

    layout->addWidget(btnSphere);
    layout->addWidget(btnBox);
    layout->addWidget(btnClear);
    layout->addStretch();

    // Connect signals
    QObject::connect(btnSphere, &QPushButton::clicked, [state]() {
        state->viewer->addSphere("sphere_" + std::to_string(state->objectCounter++),
                                Vec3(1, 0, 0), 0.5, randomPosition());
    });
    QObject::connect(btnBox, &QPushButton::clicked, [state]() {
        state->viewer->addBox("box_" + std::to_string(state->objectCounter++),
                             Vec3(0, 1, 0), 1.0, 1.0, 1.0, randomPosition());
    });
    QObject::connect(btnClear, &QPushButton::clicked, [state]() {
        state->viewer->setLayer({}, "default");
    });

    return panel;
}

// Add initial 3D objects
void addInitialObjects(EmbeddedViewer& viewer) {
    viewer.addSphere("sphere", Vec3(1, 0, 0), 1.0, Vec3(2, 0, 0))
         .addBox("box", Vec3(0, 1, 0), 1.5, 1.5, 1.5, Vec3(-2, 0, 0));
}

// Setup window layout
void setupLayout(QMainWindow& window, AppState* state) {
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);

    layout->addWidget(createButtonPanel(state));
    layout->addWidget(state->viewer->widget());

    window.setCentralWidget(container);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create main window
    QMainWindow window;
    window.setWindowTitle("Embedded Viewer Example");
    window.resize(800, 600);

    // Create state and embedded viewer
    AppState state;
    QWidget* container = &window;  // temporary parent
    auto viewer = OctoFlexViewer::createEmbedded(container);
    state.viewer = &viewer;

    // Initialize scene
    addInitialObjects(viewer);

    // Setup layout
    setupLayout(window, &state);

    // Show and run
    window.show();
    return app.exec();
}
