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

#ifndef BUILTIN_TEXTURES_H
#define BUILTIN_TEXTURES_H

#include "texture_image.h"
#include "octo_flex_export.h"

namespace octo_flex {

/**
 * @brief Get the default built-in texture
 *
 * Returns a procedurally-generated texture that can be used in examples
 * without requiring external image files.
 *
 * The texture is a colorful pattern suitable for demonstrating
 * texture mapping and transformations.
 *
 * @param width Texture width in pixels (default: 256)
 * @param height Texture height in pixels (default: 256)
 * @return TextureImage containing RGBA pixel data
 *
 * @example
 * auto texture = getBuiltinTexture();
 * viewer.add(ObjectBuilder::begin("textured_quad")
 *     .texturedQuad(texture, 3.0, 3.0, 0.5)
 *     .build(), "layer");
 */
OCTO_FLEX_VIEW_API TextureImage getBuiltinTexture(int width = 256, int height = 256);

/**
 * @brief Get a checkerboard pattern texture
 *
 * @param width Texture width in pixels
 * @param height Texture height in pixels
 * @param checker_size Size of each checker square in pixels
 * @return TextureImage containing RGBA pixel data
 */
OCTO_FLEX_VIEW_API TextureImage getCheckerboardTexture(int width = 256, int height = 256, int checker_size = 32);

/**
 * @brief Get a gradient texture
 *
 * @param width Texture width in pixels
 * @param height Texture height in pixels
 * @return TextureImage containing RGBA pixel data
 */
OCTO_FLEX_VIEW_API TextureImage getGradientTexture(int width = 256, int height = 256);

/**
 * @brief Get the embedded logo texture
 *
 * Returns the PLANNINGSCENEMANAGER logo that was previously loaded from
 * assets/texture.png. This is a 256x256 embedded version that requires
 * no file I/O.
 *
 * @return TextureImage containing the logo (256x256 RGBA)
 *
 * @example
 * auto logo = getBuiltinLogo();
 * viewer.add(ObjectBuilder::begin("logo")
 *     .texturedQuad(logo, 3.0, 3.0, 1.0)
 *     .build(), "layer");
 */
OCTO_FLEX_VIEW_API TextureImage getBuiltinLogo();

}  // namespace octo_flex

#endif  // BUILTIN_TEXTURES_H
