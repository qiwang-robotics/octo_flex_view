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

#include "object_builder.h"

#include <cmath>
#include <iostream>

#include <QImage>
#include <QString>

#include "shape.h"
#include "textured_quad.h"
#include "utils.h"

namespace octo_flex {

// ============================================================================
// Constructor and Static Factory
// ============================================================================

ObjectBuilder::ObjectBuilder(const std::string& id)
    : object_(std::make_shared<Object>(id)), text_color_(0.2, 0.2, 0.2) {}

ObjectBuilder ObjectBuilder::begin(const std::string& id) { return ObjectBuilder(id); }

// ============================================================================
// Geometric Primitives
// ============================================================================

ObjectBuilder& ObjectBuilder::sphere(const Vec3& color, double radius, bool transparent) {
    // Generate sphere shape and add to object
    auto sphere_obj = generateSphere("temp", color, radius, transparent);
    for (const auto& shape : sphere_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::box(const Vec3& color, double width, double height, double depth, bool transparent) {
    auto box_obj = generateCubic("temp", color, width, height, depth, transparent);
    for (const auto& shape : box_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::cylinder(const Vec3& color, double radius, double height, bool transparent) {
    auto cylinder_obj = generateCylinder("temp", color, radius, height, transparent);
    for (const auto& shape : cylinder_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::cone(const Vec3& color, double radius, double height, bool transparent) {
    auto cone_obj = generateCone("temp", color, radius, height, transparent);
    for (const auto& shape : cone_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::pyramid(const Vec3& color, double width, double height, double depth, bool transparent) {
    auto pyramid_obj = generatePyramid("temp", color, width, height, depth, transparent);
    for (const auto& shape : pyramid_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::arrow(const Vec3& color, double length, double shaft_radius, double head_width,
                                    double head_length, bool transparent) {
    auto arrow_obj = generateArrow("temp", color, length, shaft_radius, head_width, head_length, transparent);
    for (const auto& shape : arrow_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::simpleArrow(const Vec3& color, double length, double head_width, double head_length,
                                          double line_width, bool transparent) {
    // Use defaults if not specified (negative values indicate default)
    if (head_width < 0) {
        head_width = 0.2 * length;
    }
    if (head_length < 0) {
        head_length = 0.3 * length;
    }

    auto arrow_obj = generateSimpleArrow("temp", color, length, head_width, head_length, line_width, transparent);
    for (const auto& shape : arrow_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::quad(const Vec3& color, double width, double height, bool transparent) {
    auto quad_obj = generateQuad("temp", color, width, height, transparent);
    for (const auto& shape : quad_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::ellipsoid(const Vec3& color, double radius_x, double radius_y, double radius_z,
                                        bool transparent) {
    auto ellipsoid_obj = generateEllipsoid("temp", color, radius_x, radius_y, radius_z, transparent);
    for (const auto& shape : ellipsoid_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

ObjectBuilder& ObjectBuilder::capsule(const Vec3& color, double radius, double height, bool transparent) {
    auto capsule_obj = generateCapsule("temp", color, radius, height, transparent);
    for (const auto& shape : capsule_obj->shapes()) {
        applyPendingShapeTransform(shape);  // Phase 1 - applies WITHOUT resetting
        object_->addShape(shape);
    }
    resetPendingShapeTransform();  // Reset after all shapes from this primitive
    return *this;
}

// ============================================================================
// Basic Shapes (Points, Lines, Dash, Loop, Polygon)
// ============================================================================

ObjectBuilder& ObjectBuilder::points(const std::vector<Vec3>& points, const Vec3& color, double point_size,
                                     bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Points, point_size, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, color);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::points(const std::vector<Vec3>& points, const std::vector<Vec3>& colors,
                                     double point_size, bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Points, point_size, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, colors);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::lines(const std::vector<Vec3>& points, const Vec3& color, double line_width,
                                    bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Lines, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, color);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::lines(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, double line_width,
                                    bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Lines, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, colors);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::dashedLines(const std::vector<Vec3>& points, const Vec3& color, double line_width,
                                          bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Dash, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, color);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::dashedLines(const std::vector<Vec3>& points, const std::vector<Vec3>& colors,
                                          double line_width, bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Dash, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, colors);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::loop(const std::vector<Vec3>& points, const Vec3& color, double line_width,
                                   bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Loop, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, color);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::loop(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, double line_width,
                                   bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Loop, line_width, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, colors);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::polygon(const std::vector<Vec3>& points, const Vec3& color, bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Polygon, 1.0, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, color);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

ObjectBuilder& ObjectBuilder::polygon(const std::vector<Vec3>& points, const std::vector<Vec3>& colors,
                                      bool transparent) {
    auto shape = std::make_shared<Shape>(Shape::Polygon, 1.0, transparent ? 0.8 : 1.0);
    shape->setPointsWithColor(points, colors);
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

// ============================================================================
// Texture Support
// ============================================================================

ObjectBuilder& ObjectBuilder::texturedQuad(double width, double height, const std::string& texture_path,
                                           double transparency) {
    // Store texture path for deferred loading (will be applied in build())
    pending_textures_.push_back({texture_path, TextureImage(), false, width, height, transparency});
    return *this;
}

ObjectBuilder& ObjectBuilder::texturedQuad(const TextureImage& image, double width, double height,
                                           double transparency) {
    // Store texture image for deferred loading (will be applied in build())
    pending_textures_.push_back({"", image, true, width, height, transparency});
    return *this;
}

void ObjectBuilder::applyPendingTextures() {
    for (const auto& tex : pending_textures_) {
        TexturedQuad::Image image;

        if (tex.use_image) {
            // Use provided image data directly
            image = tex.image;

            if (!image.isValid()) {
                std::cerr << "Error: Invalid texture image data provided" << std::endl;
                continue;  // Skip this texture
            }
        } else {
            // Load image from file using Qt
            QImage source(QString::fromStdString(tex.path));

            if (source.isNull()) {
                std::cerr << "Error: Failed to load texture from file: " << tex.path << std::endl;
                continue;  // Skip this texture
            }

            // Convert to RGBA format and mirror vertically (OpenGL texture coordinate convention)
            QImage formatted = source.convertToFormat(QImage::Format_RGBA8888).mirrored();

            // Create Image structure
            image.width = formatted.width();
            image.height = formatted.height();

            // Copy pixel data
            const size_t byte_count = static_cast<size_t>(image.width) * image.height * 4;
            image.pixels.assign(formatted.bits(), formatted.bits() + byte_count);
        }

        // Create textured quad and load image
        auto textured_quad = std::make_shared<TexturedQuad>(tex.width, tex.height);
        textured_quad->setTransparency(tex.transparency);
        textured_quad->loadTextureFromImage(image);

        // Phase 1: Apply shape-level transforms (new system)
        applyPendingShapeTransform(textured_quad);

        // Legacy: Apply old pending transforms to this deferred shape (for backward compatibility)
        for (const auto& transform : pending_transforms_) {
            if (transform.type == PendingTransform::Move) {
                textured_quad->move(transform.move);
            } else {
                textured_quad->rotate(transform.rotate);
            }
        }

        // Add to object
        object_->addShape(textured_quad);
    }
}

// ============================================================================
// Custom Shapes
// ============================================================================

ObjectBuilder& ObjectBuilder::addShape(std::shared_ptr<Shape> shape) {
    applyPendingShapeTransform(shape);  // Phase 1
    object_->addShape(shape);
    resetPendingShapeTransform();  // Reset after single shape
    return *this;
}

// ============================================================================
// Transformations
// ============================================================================

ObjectBuilder& ObjectBuilder::at(const Vec3& position) {
    // Store transform for deferred shapes (texturedQuad)
    pending_transforms_.push_back({PendingTransform::Move, position, Quaternion()});
    // Also apply to object for immediate shapes (sphere, box, etc.)
    object_->move(position);
    return *this;
}

ObjectBuilder& ObjectBuilder::at(double x, double y, double z) { return at(Vec3(x, y, z)); }

ObjectBuilder& ObjectBuilder::rotateX(double radians) {
    Quaternion quat = octo_flex::rotateX(radians);
    pending_transforms_.push_back({PendingTransform::Rotate, Vec3(), quat});
    object_->rotate(quat);
    return *this;
}

ObjectBuilder& ObjectBuilder::rotateY(double radians) {
    Quaternion quat = octo_flex::rotateY(radians);
    pending_transforms_.push_back({PendingTransform::Rotate, Vec3(), quat});
    object_->rotate(quat);
    return *this;
}

ObjectBuilder& ObjectBuilder::rotateZ(double radians) {
    Quaternion quat = octo_flex::rotateZ(radians);
    pending_transforms_.push_back({PendingTransform::Rotate, Vec3(), quat});
    object_->rotate(quat);
    return *this;
}

ObjectBuilder& ObjectBuilder::rotate(const Quaternion& quat) {
    pending_transforms_.push_back({PendingTransform::Rotate, Vec3(), quat});
    object_->rotate(quat);
    return *this;
}

// ============================================================================
// Metadata
// ============================================================================

ObjectBuilder& ObjectBuilder::withInfo(const std::string& info) {
    info_text_ = info;
    return *this;
}

ObjectBuilder& ObjectBuilder::withColor(const Vec3& text_color) {
    text_color_ = text_color;
    has_text_color_ = true;
    return *this;
}

// ============================================================================
// Shape-Level Transformations (Phase 1)
// ============================================================================

ObjectBuilder& ObjectBuilder::shapeAt(double x, double y, double z) {
    next_shape_position_ = Vec3(x, y, z);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeAt(const Vec3& position) {
    next_shape_position_ = position;
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeRotateX(double radians) {
    // Use global rotateX function from utils (not ObjectBuilder::rotateX)
    Quaternion q = ::octo_flex::rotateX(radians);
    next_shape_orientation_ = quaternionMultiply(next_shape_orientation_, q);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeRotateY(double radians) {
    // Use global rotateY function from utils (not ObjectBuilder::rotateY)
    Quaternion q = ::octo_flex::rotateY(radians);
    next_shape_orientation_ = quaternionMultiply(next_shape_orientation_, q);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeRotateZ(double radians) {
    // Use global rotateZ function from utils (not ObjectBuilder::rotateZ)
    Quaternion q = ::octo_flex::rotateZ(radians);
    next_shape_orientation_ = quaternionMultiply(next_shape_orientation_, q);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeRotate(const Quaternion& quat) {
    next_shape_orientation_ = quaternionMultiply(next_shape_orientation_, quat);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeScale(double sx, double sy, double sz) {
    next_shape_scale_ = Vec3(sx, sy, sz);
    return *this;
}

ObjectBuilder& ObjectBuilder::shapeScale(double uniform_scale) {
    return shapeScale(uniform_scale, uniform_scale, uniform_scale);
}

// ============================================================================
// Internal Helpers (Phase 1)
// ============================================================================

namespace {
// Helper: Check if quaternion is identity (no rotation)
bool isIdentityQuaternion(const Quaternion& q) {
    const double eps = 1e-6;
    return (std::abs(q.x) < eps && std::abs(q.y) < eps && std::abs(q.z) < eps && std::abs(std::abs(q.w) - 1.0) < eps);
}
}  // anonymous namespace

void ObjectBuilder::applyPendingShapeTransform(std::shared_ptr<Shape> shape) {
    if (!shape) return;

    // Apply transforms in order: Scale -> Rotate -> Translate

    // 1. Apply scaling (first)
    const double eps = 1e-9;
    if (std::abs(next_shape_scale_.x - 1.0) > eps || std::abs(next_shape_scale_.y - 1.0) > eps ||
        std::abs(next_shape_scale_.z - 1.0) > eps) {
        shape->scale(next_shape_scale_.x, next_shape_scale_.y, next_shape_scale_.z);
    }

    // 2. Apply rotation
    if (!isIdentityQuaternion(next_shape_orientation_)) {
        shape->rotate(next_shape_orientation_);
    }

    // 3. Apply translation (last)
    if (std::abs(next_shape_position_.x) > eps || std::abs(next_shape_position_.y) > eps ||
        std::abs(next_shape_position_.z) > eps) {
        shape->move(next_shape_position_);
    }

    // Note: Do NOT reset here - reset is done by caller after all shapes processed
}

void ObjectBuilder::resetPendingShapeTransform() {
    next_shape_position_ = Vec3(0, 0, 0);
    next_shape_orientation_ = Quaternion();
    next_shape_scale_ = Vec3(1, 1, 1);
}

// ============================================================================
// Build
// ============================================================================

std::shared_ptr<Object> ObjectBuilder::build() {
    // Apply pending textures
    applyPendingTextures();

    // Apply info text if set
    if (!info_text_.empty()) {
        if (has_text_color_) {
            object_->setInfo(info_text_, text_color_);
        } else {
            object_->setInfo(info_text_);
        }
    }

    // Return the built object
    return object_;
}

}  // namespace octo_flex
