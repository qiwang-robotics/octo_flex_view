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


#include "shape.h"
#include <cmath>
#include "utils.h"

namespace octo_flex {
Shape::Shape() : editable_(true){};
Shape::Shape(ShapeType type, double width, double transparency) : editable_(true) {
    type_ = type;
    width_ = width;
    transparency_ = transparency;
}

void Shape::setPointsWithColor(const std::vector<Vec3>& points, const Vec3& color) {
    if (!editable_) return;
    points_ = points;
    color_ = color;
    colors_.clear();
}

void Shape::setPointsWithColor(const std::vector<Vec3>& points, const std::vector<Vec3>& colors) {
    if (!editable_) return;
    points_.assign(points.begin(), points.end());
    colors_.assign(colors.begin(), colors.end());
}

bool Shape::isEditable() const { return editable_; }
void Shape::setInEditable() { editable_ = false; }

const Vec3& Shape::color(size_t i) const {
    if (i + 1 > colors_.size()) {
        return color_;
    } else {
        return colors_[i];
    }
}

const std::vector<Vec3>& Shape::points() const { return points_; }
const std::vector<Vec3>& Shape::colors() const { return colors_; }
const Vec3& Shape::color() const { return color_; }

Shape::ShapeType Shape::type() const { return type_; }
void Shape::setType(ShapeType type) {
    if (!editable_) return;
    type_ = type;
}

double Shape::width() const { return width_; }
void Shape::setWidth(double width) {
    if (!editable_) return;
    width_ = width;
}

double Shape::transparency() const { return transparency_; }
void Shape::setTransparency(double transparency) {
    if (!editable_) return;
    transparency_ = transparency;
}

Shape::Ptr Shape::clone() {
    auto new_shape = std::make_shared<Shape>(type_, width_, transparency_);
    new_shape->setPointsWithColor(points_, colors_);
    new_shape->color_ = color_;
    return new_shape;
}

void Shape::move(const Vec3& vec) {
    if (!editable_) return;
    for (auto& point : points_) {
        point = point + vec;
    }
}
void Shape::rotate(const Quaternion& quad) {
    if (!editable_) return;
    // Apply rotation to each point
    for (auto& point : points_) {
        point = quaternionRotateVector(quad, point);
    }
}

void Shape::scale(double sx, double sy, double sz) {
    if (!editable_) return;
    // Apply scaling to each point
    for (auto& point : points_) {
        point.x *= sx;
        point.y *= sy;
        point.z *= sz;
    }
}

void Shape::scale(const Vec3& scale_factors) { scale(scale_factors.x, scale_factors.y, scale_factors.z); }
}  // namespace octo_flex
