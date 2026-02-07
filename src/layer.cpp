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


#include "layer.h"

namespace octo_flex {
const std::string& Layer::id() const { return id_; }

const ObjectList Layer::objects() {
    ObjectList objects;

    std::unique_lock<std::mutex> lock(mtx_);
    objects = objects_;
    return objects;
}

void Layer::addObject(Object::Ptr obj) {
    if (obj == nullptr) return;
    std::unique_lock<std::mutex> lock(mtx_);

    // If an object with the same ID exists, move it to outdated list
    auto it = objects_.find(obj->id());
    if (it != objects_.end() && it->second != obj) {
        outdated_objects_.push_back(it->second);
    }

    objects_[obj->id()] = obj;
}

void Layer::removeObject(std::string& object_id) {
    std::unique_lock<std::mutex> lock(mtx_);
    auto it = objects_.find(object_id);
    if (it != objects_.end()) {
        objects_.erase(it);
    }
}

Object::Ptr Layer::findObject(std::string id) {
    Object::Ptr obj = nullptr;

    std::unique_lock<std::mutex> lock(mtx_);
    auto it = objects_.find(id);
    if (it != objects_.end()) {
        obj = it->second;
    }
    return obj;
}

void Layer::clear() {
    std::unique_lock<std::mutex> lock(mtx_);
    objects_.clear();
}

void Layer::setObjects(const std::vector<Object::Ptr>& objects) {
    std::unique_lock<std::mutex> lock(mtx_);
    objects_.clear();
    for (auto& obj : objects) {
        if (obj != nullptr) {
            objects_[obj->id()] = obj;
        }
    }
}

std::vector<Object::Ptr> Layer::collectOutdatedObjects() {
    std::unique_lock<std::mutex> lock(mtx_);
    std::vector<Object::Ptr> result;
    result.swap(outdated_objects_);  // Move and clear in one operation
    return result;
}

}  // namespace octo_flex
