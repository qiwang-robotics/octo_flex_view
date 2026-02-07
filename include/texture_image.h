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

#ifndef OCTO_FLEX_TEXTURE_IMAGE_H
#define OCTO_FLEX_TEXTURE_IMAGE_H

#include <vector>

#include "octo_flex_export.h"

namespace octo_flex {

/**
 * @brief RGBA texture image data structure
 *
 * This structure allows users to create custom texture images programmatically.
 * The image data must be in RGBA format with 4 bytes per pixel.
 *
 * **Recommended Usage**: Pass this to ObjectBuilder::texturedQuad() to create
 * textured objects with custom image data.
 *
 * **Pixel Format**: RGBA, 4 bytes per pixel (Red, Green, Blue, Alpha)
 * **Memory Layout**: pixels[0..3] = first pixel RGBA,
 *                    pixels[4..7] = second pixel RGBA, etc.
 *
 * @example Creating a solid red image:
 * @code
 * #include "textured_quad.h"
 * #include "object_builder.h"
 *
 * // Create 256x256 solid red image
 * octo_flex::TextureImage image;
 * image.width = 256;
 * image.height = 256;
 * image.pixels.resize(256 * 256 * 4);
 *
 * for (size_t i = 0; i < image.pixels.size(); i += 4) {
 *     image.pixels[i + 0] = 255;  // R
 *     image.pixels[i + 1] = 0;    // G
 *     image.pixels[i + 2] = 0;    // B
 *     image.pixels[i + 3] = 255;  // A
 * }
 *
 * // Use with ObjectBuilder
 * auto obj = ObjectBuilder::begin("custom_texture")
 *     .texturedQuad(image, 3.0, 3.0, 0.8)
 *     .build();
 * @endcode
 *
 * @example Creating a gradient image:
 * @code
 * // Create 256x256 horizontal red gradient
 * octo_flex::TextureImage createGradient() {
 *     octo_flex::TextureImage image;
 *     image.width = 256;
 *     image.height = 256;
 *     image.pixels.resize(256 * 256 * 4);
 *
 *     for (int y = 0; y < 256; y++) {
 *         for (int x = 0; x < 256; x++) {
 *             int i = (y * 256 + x) * 4;
 *             image.pixels[i + 0] = x;        // R: horizontal gradient
 *             image.pixels[i + 1] = 0;        // G
 *             image.pixels[i + 2] = 0;        // B
 *             image.pixels[i + 3] = 255;      // A
 *         }
 *     }
 *
 *     return image;
 * }
 *
 * auto image = createGradient();
 * auto obj = ObjectBuilder::begin("gradient")
 *     .texturedQuad(image, 3.0, 3.0)
 *     .build();
 * @endcode
 *
 * @example Procedural texture generation:
 * @code
 * // Create checkerboard pattern
 * octo_flex::TextureImage createCheckerboard(int size, int squares) {
 *     octo_flex::TextureImage image;
 *     image.width = size;
 *     image.height = size;
 *     image.pixels.resize(size * size * 4);
 *
 *     int squareSize = size / squares;
 *
 *     for (int y = 0; y < size; y++) {
 *         for (int x = 0; x < size; x++) {
 *             int i = (y * size + x) * 4;
 *             bool isWhite = ((x / squareSize) + (y / squareSize)) % 2 == 0;
 *
 *             unsigned char color = isWhite ? 255 : 0;
 *             image.pixels[i + 0] = color;    // R
 *             image.pixels[i + 1] = color;    // G
 *             image.pixels[i + 2] = color;    // B
 *             image.pixels[i + 3] = 255;      // A
 *         }
 *     }
 *
 *     return image;
 * }
 *
 * auto checker = createCheckerboard(512, 8);
 * auto obj = ObjectBuilder::begin("checkerboard")
 *     .texturedQuad(checker, 4.0, 4.0)
 *     .build();
 * @endcode
 */
struct OCTO_FLEX_VIEW_API TextureImage {
    int width = 0;                          ///< Image width in pixels
    int height = 0;                         ///< Image height in pixels
    std::vector<unsigned char> pixels;      ///< RGBA pixel data (4 bytes per pixel)

    /**
     * @brief Check if image data is valid
     * @return true if width > 0, height > 0, and pixel data size matches dimensions
     *
     * A valid image must have:
     * - width > 0
     * - height > 0
     * - pixels.size() == width * height * 4 (RGBA format)
     */
    bool isValid() const {
        return width > 0 && height > 0 && pixels.size() == static_cast<size_t>(width * height * 4);
    }
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_TEXTURE_IMAGE_H
