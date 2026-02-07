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
 * @file simple_viewer.cpp
 * @brief Minimal standalone viewer example
 */

#include <iostream>

#include "builtin_textures.h"
#include "object_builder.h"
#include "octo_flex_viewer.h"

using namespace octo_flex;

// Add basic geometry objects
void addGeometry(OctoFlexViewer& viewer) {
    viewer.add(ObjectBuilder::begin("sphere")
                   .sphere(Vec3(0, 1, 0), 1.0)
                   .at(3, 0, 0)
                   .build())
        .add(ObjectBuilder::begin("cone")
                 .cone(Vec3(1, 0, 0), 1.0, 2.0)
                 .at(0, 0, 0)
                 .build())
        .add(ObjectBuilder::begin("cylinder")
                 .cylinder(Vec3(1, 0, 1), 0.5, 1.5)
                 .at(-3, 0, 0)
                 .build());
}

// Add textured objects
void addTexturedObjects(OctoFlexViewer& viewer) {
    viewer.add(ObjectBuilder::begin("logo")
                   .texturedQuad(getBuiltinLogo(), 3.0, 3.0, 1.0)
                   .at(0, 3, 0)
                   .build())
        .add(ObjectBuilder::begin("checker")
                 .texturedQuad(getCheckerboardTexture(), 2.5, 2.5, 0.8)
                 .at(0, -3, 0)
                 .build());
}

int main(int argc, char* argv[]) {
    // Create viewer with auto-managed window and event loop
    auto viewer = OctoFlexViewer::create("Simple Viewer", 1024, 768);

    // Setup scene after OpenGL context is ready
    auto setup = [](OctoFlexViewer& v) {
        std::cout << "Creating scene..." << std::endl;
        addGeometry(v);
        addTexturedObjects(v);
        std::cout << "Scene ready!" << std::endl;
    };

    // Show window and run (blocking event loop)
    viewer.show();
    return viewer.run(setup);
}
