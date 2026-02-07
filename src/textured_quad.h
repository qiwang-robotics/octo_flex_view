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


#ifndef TEXTURED_QUAD_H
#define TEXTURED_QUAD_H


#include <array>
#include <string>
#include <vector>

#include <GL/gl.h>

#include "def.h"
#include "shape.h"
#include "texture_image.h"  // Include public TextureImage definition

namespace octo_flex {

class TexturedQuad : public Shape {
   public:
    struct UV {
        float u;
        float v;
    };

    // Use the public TextureImage type
    using Image = TextureImage;

    typedef std::shared_ptr<TexturedQuad> Ptr;

   public:
    TexturedQuad(double width, double height);
    ~TexturedQuad() override;

    bool loadTextureFromImage(const Image& image);
    void setSize(double width, double height);

    void setUVs(const std::array<UV, 4>& uvs);
    const std::array<UV, 4>& uvs() const;

    GLuint textureId() const;
    bool hasTexture() const;
    int textureWidth() const;
    int textureHeight() const;

    Shape::Ptr clone() override;
    void releaseResources() override;

   private:
    void updateQuadPoints();
    bool cloneTexture(GLuint source_texture_id, int width, int height, GLuint& dest_texture_id);
    void safeDeleteTexture();
    void ensureTextureUploaded() const;  // const because it's lazy initialization

    double width_;
    double height_;
    mutable GLuint texture_id_;  // mutable for lazy GPU upload
    int texture_width_;
    int texture_height_;
    std::array<UV, 4> uvs_;
    Image image_data_;  // Store image data for lazy GPU upload
};

}  // namespace octo_flex

#endif /* TEXTURED_QUAD_H */
