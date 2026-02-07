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
 * @file builtin_textures.cpp
 * @brief Implementation of built-in texture generation
 */

#include "builtin_textures.h"
#include <cmath>
#include <iostream>

#include "builtin_logo_data.h"

// Import Qt image format plugins - commented out to avoid static linking issues
// #include <QtPlugin>
// Q_IMPORT_PLUGIN(QJpegPlugin)

namespace octo_flex {

TextureImage getBuiltinTexture(int width, int height) {
    TextureImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(width * height * 4);  // RGBA format

    // Create a colorful radial gradient with geometric pattern
    const float center_x = width / 2.0f;
    const float center_y = height / 2.0f;
    const float max_dist = std::sqrt(center_x * center_x + center_y * center_y);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 4;

            // Calculate distance from center
            float dx = x - center_x;
            float dy = y - center_y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx);

            // Create radial pattern
            float radial = dist / max_dist;
            float angular = (std::sin(angle * 8) + 1.0f) / 2.0f;  // 8 segments

            // Combine patterns for interesting effect
            float r = radial * 0.6f + angular * 0.4f;
            float g = (1.0f - radial) * 0.5f + std::cos(angle * 6) * 0.3f + 0.2f;
            float b = angular * 0.5f + 0.3f;

            // Add some circles for visual interest
            float ring = std::fmod(dist, 30.0f) / 30.0f;
            if (ring > 0.9f) {
                r += 0.2f;
                g += 0.2f;
                b += 0.2f;
            }

            // Clamp and convert to byte
            image.pixels[i + 0] = static_cast<unsigned char>(std::min(r * 255.0f, 255.0f));
            image.pixels[i + 1] = static_cast<unsigned char>(std::min(g * 255.0f, 255.0f));
            image.pixels[i + 2] = static_cast<unsigned char>(std::min(b * 255.0f, 255.0f));
            image.pixels[i + 3] = 255;  // Fully opaque
        }
    }

    return image;
}

TextureImage getCheckerboardTexture(int width, int height, int checker_size) {
    TextureImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(width * height * 4);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 4;
            bool is_white = ((x / checker_size) + (y / checker_size)) % 2 == 0;
            unsigned char color = is_white ? 220 : 60;

            image.pixels[i + 0] = color;
            image.pixels[i + 1] = color;
            image.pixels[i + 2] = color;
            image.pixels[i + 3] = 255;
        }
    }

    return image;
}

TextureImage getGradientTexture(int width, int height) {
    TextureImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(width * height * 4);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 4;
            image.pixels[i + 0] = static_cast<unsigned char>((x * 255) / width);   // R: horizontal
            image.pixels[i + 1] = static_cast<unsigned char>((y * 255) / height);  // G: vertical
            image.pixels[i + 2] = static_cast<unsigned char>(128);                 // B: constant
            image.pixels[i + 3] = 255;                                             // A: opaque
        }
    }

    return image;
}

// ============================================================================
// Logo Texture (embedded C++ data)
// ============================================================================

TextureImage getBuiltinLogo() {
    TextureImage image;
    image.width = LOGO_WIDTH;
    image.height = LOGO_HEIGHT;
    image.pixels.assign(LOGO_DATA, LOGO_DATA + (LOGO_WIDTH * LOGO_HEIGHT * 4));
    return image;
}

}  // namespace octo_flex
