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
 * @file main.cpp
 * @brief Dynamic 3D bar chart example
 *
 * Generates a grid of bars whose heights animate over time.
 */

#include <QElapsedTimer>
#include <QTimer>
#include <algorithm>
#include <cmath>

#include "object_builder.h"
#include "octo_flex_viewer.h"

using namespace octo_flex;

// ============================================================================
// Chart Configuration
// ============================================================================
struct ChartConfig {
    int rows = 8;
    int cols = 12;
    double spacing = 1.0;
    double barWidth = 0.65;
    double barDepth = 0.65;
    double baseHeight = 0.3;
    double amplitude = 2.6;
};

// ============================================================================
// Color Utilities
// ============================================================================
namespace {
Vec3 mix(const Vec3& a, const Vec3& b, double t) {
    double clamped = std::max(0.0, std::min(1.0, t));
    return a * (1.0 - clamped) + b * clamped;
}

Vec3 heightToColor(double normalizedHeight) {
    const Vec3 low(0.1, 0.4, 0.9);   // Blue
    const Vec3 mid(0.2, 0.85, 0.5);  // Green
    const Vec3 high(0.9, 0.35, 0.2); // Orange

    if (normalizedHeight < 0.5) {
        return mix(low, mid, normalizedHeight * 2.0);
    }
    return mix(mid, high, (normalizedHeight - 0.5) * 2.0);
}
}  // namespace

// ============================================================================
// Bar Update Logic
// ============================================================================
void updateBars(OctoFlexViewer& viewer, const ChartConfig& config, double time) {
    const double totalWidth = (config.cols - 1) * config.spacing;
    const double totalDepth = (config.rows - 1) * config.spacing;
    const double startX = -totalWidth * 0.5;
    const double startY = -totalDepth * 0.5;

    for (int r = 0; r < config.rows; ++r) {
        for (int c = 0; c < config.cols; ++c) {
            // Calculate wave height
            double phase = time * 1.4 + r * 0.45 + c * 0.3;
            double wave = (std::sin(phase) + 1.0) * 0.5;
            double height = config.baseHeight + config.amplitude * wave;

            // Color based on height
            double colorT = (height - config.baseHeight) / config.amplitude;
            Vec3 color = heightToColor(colorT);

            // Position
            double x = startX + c * config.spacing;
            double y = startY + r * config.spacing;
            double z = height * 0.5;

            // Create and add bar
            std::string id = "bar_chart#bar#" + std::to_string(r) + "_" + std::to_string(c);
            auto bar = ObjectBuilder::begin(id)
                           .box(color, config.barWidth, height, config.barDepth, true)
                           .at(x, y, z)
                           .build();
            viewer.add(bar);
        }
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================
int main(int argc, char* argv[]) {
    // Create viewer
    auto viewer = OctoFlexViewer::create("Dynamic 3D Bar Chart", 1280, 720, &argc, &argv);

    // Configuration
    ChartConfig config;

    // Setup animation timer
    QElapsedTimer clock;
    clock.start();

    auto* timer = new QTimer(nullptr);
    QObject::connect(timer, &QTimer::timeout, [&viewer, &config, &clock]() {
        double time = clock.elapsed() * 0.001;
        updateBars(viewer, config, time);
    });
    timer->start(33);  // ~30 FPS

    // Run
    viewer.show();
    return viewer.run();
}
