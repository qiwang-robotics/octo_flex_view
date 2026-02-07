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


#include "object.h"

#include "def.h"
#include "utils.h"

namespace octo_flex {
Object::Object(ObjectId id) : editable_(true), id_(id) { setInfo(id); }

bool Object::isEditable() const { return editable_; }
void Object::setInEditable() {
    editable_ = false;
    for (const auto& shape : shapes_) {
        if (shape) {
            shape->setInEditable();
        }
    }
}

void Object::addShape(Shape::Ptr shape) {
    if (!editable_) return;
    shapes_.push_back(shape);
}

void Object::resetTransform() {
    if (!editable_) return;
    position_ = Vec3();
    orientation_ = Quaternion();
}

void Object::merge(const Object::Ptr obj) {
    if (!editable_ || !obj || !obj->isEditable()) return;
    shapes_.insert(shapes_.end(), obj->shapes_.begin(), obj->shapes_.end());
    obj->shapes_.clear();
}

Object::Ptr Object::clone() {
    Object::Ptr new_obj = std::make_shared<Object>(id_);

    for (size_t i = 0; i < shapes_.size(); ++i) {
        new_obj->shapes_.push_back(shapes_[i]->clone());
    }
    new_obj->position_ = position_;
    new_obj->orientation_ = orientation_;
    new_obj->info_ = info_;
    new_obj->detail_ = detail_;
    new_obj->text_color_ = text_color_;

    return new_obj;
}

const Vec3& Object::position() const { return position_; }
const Quaternion& Object::orientation() const { return orientation_; }

Vec3 Object::textColor() const { return text_color_; }

const std::string& Object::info() const { return info_; }
void Object::setInfo(const std::string& info, const Vec3 color) {
    if (!editable_) return;
    info_ = info;
    text_color_ = color;
}

const std::vector<Shape::Ptr>& Object::shapes() const { return shapes_; }

void Object::move(const Vec3& vec) {
    if (!editable_) return;
    for (auto& shape : shapes_) {
        shape->move(vec);
    }
    position_ = vec + position_;
}

void Object::rotate(const Quaternion& quad) {
    if (!editable_) return;
    for (auto& shape : shapes_) {
        shape->rotate(quad);
    }
    position_ = quaternionRotateVector(quad, position_);
    orientation_ = quaternionMultiply(quad, orientation_);
}
}  // namespace octo_flex
