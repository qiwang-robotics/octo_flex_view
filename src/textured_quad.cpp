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


#include "textured_quad.h"

#include <QDebug>
#include <QOpenGLContext>

#include <vector>

namespace octo_flex {

TexturedQuad::TexturedQuad(double width, double height)
    : Shape(Shape::TexturedQuad, 1.0, 1.0),
      width_(width),
      height_(height),
      texture_id_(0),
      texture_width_(0),
      texture_height_(0) {
    // UV coordinates are mirrored horizontally to correct left-right reversal
    // when viewing from top view. Top view uses camera up vector (0, -1, 0)
    // which inverts the Y-axis on screen, causing the default UV mapping
    // to appear mirrored.
    uvs_ = {UV{1.0f, 0.0f}, UV{0.0f, 0.0f}, UV{0.0f, 1.0f}, UV{1.0f, 1.0f}};
    updateQuadPoints();
}

TexturedQuad::~TexturedQuad() { safeDeleteTexture(); }

void TexturedQuad::updateQuadPoints() {
    const float half_w = static_cast<float>(width_ * 0.5);
    const float half_h = static_cast<float>(height_ * 0.5);
    std::vector<Vec3> points = {
        Vec3(-half_w, -half_h, 0.0f),
        Vec3(half_w, -half_h, 0.0f),
        Vec3(half_w, half_h, 0.0f),
        Vec3(-half_w, half_h, 0.0f),
    };
    setPointsWithColor(points, Vec3(1.0f, 1.0f, 1.0f));
}

void TexturedQuad::setSize(double width, double height) {
    if (!isEditable()) return;
    width_ = width;
    height_ = height;
    updateQuadPoints();
}

void TexturedQuad::setUVs(const std::array<UV, 4>& uvs) {
    if (!isEditable()) return;
    uvs_ = uvs;
}

const std::array<TexturedQuad::UV, 4>& TexturedQuad::uvs() const { return uvs_; }

GLuint TexturedQuad::textureId() const {
    ensureTextureUploaded();
    return texture_id_;
}

bool TexturedQuad::hasTexture() const { return texture_id_ != 0 || image_data_.isValid(); }

// Note: Image::isValid() is now defined inline in texture_image.h

bool TexturedQuad::loadTextureFromImage(const Image& image) {
    if (!isEditable()) {
        qWarning() << "TexturedQuad::loadTextureFromImage: Shape not editable.";
        return false;
    }
    if (!image.isValid()) {
        qWarning() << "TexturedQuad::loadTextureFromImage: Invalid image data.";
        return false;
    }

    // Save image data for lazy GPU upload
    image_data_ = image;
    texture_width_ = image.width;
    texture_height_ = image.height;

    // If we have OpenGL context, upload immediately
    if (QOpenGLContext::currentContext()) {
        ensureTextureUploaded();
    }

    return true;
}

Shape::Ptr TexturedQuad::clone() {
    auto new_shape = std::make_shared<TexturedQuad>(width_, height_);
    new_shape->setUVs(uvs_);
    new_shape->setTransparency(transparency());

    // Copy image data (no OpenGL context required)
    // Each clone will create its own texture lazily when rendered
    new_shape->image_data_ = image_data_;
    new_shape->texture_width_ = texture_width_;
    new_shape->texture_height_ = texture_height_;
    // texture_id_ stays 0, will be created lazily via ensureTextureUploaded()

    return new_shape;
}

int TexturedQuad::textureWidth() const { return texture_width_; }

int TexturedQuad::textureHeight() const { return texture_height_; }

bool TexturedQuad::cloneTexture(GLuint source_texture_id, int width, int height, GLuint& dest_texture_id) {
    // Check if OpenGL context is valid
    if (!QOpenGLContext::currentContext()) {
        qWarning() << "TexturedQuad::cloneTexture: No valid OpenGL context";
        return false;
    }

    // Allocate buffer to read texture data
    std::vector<unsigned char> pixels(width * height * 4);

    // Read texture data from source
    glBindTexture(GL_TEXTURE_2D, source_texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Create new texture
    glGenTextures(1, &dest_texture_id);
    glBindTexture(GL_TEXTURE_2D, dest_texture_id);

    // Save current pixel storage alignment.
    GLint oldAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldAlignment);

    // Set texture parameters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Restore pixel storage alignment.
    glPixelStorei(GL_UNPACK_ALIGNMENT, oldAlignment);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void TexturedQuad::ensureTextureUploaded() const {
    // If texture already uploaded, nothing to do
    if (texture_id_ != 0) {
        return;
    }

    // Check if we have image data to upload
    if (!image_data_.isValid()) {
        return;
    }

    // Check if OpenGL context is valid
    if (!QOpenGLContext::currentContext()) {
        // No context yet, will upload later during rendering
        return;
    }

    // Generate texture ID
    glGenTextures(1, &texture_id_);

    // Upload texture data
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    // Save current pixel storage alignment
    GLint oldAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldAlignment);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data_.width, image_data_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data_.pixels.data());

    // Restore pixel storage alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, oldAlignment);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void TexturedQuad::releaseResources() {
    if (texture_id_ == 0) {
        return;
    }

    // This method should be called on the main thread with OpenGL context
    if (QOpenGLContext::currentContext()) {
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
        texture_width_ = 0;
        texture_height_ = 0;
    } else {
        qWarning() << "TexturedQuad::releaseResources: No OpenGL context, texture" << texture_id_
                   << "may leak. This method should be called on the rendering thread.";
    }
}

void TexturedQuad::safeDeleteTexture() {
    // Delegate to releaseResources
    releaseResources();
}

}  // namespace octo_flex
