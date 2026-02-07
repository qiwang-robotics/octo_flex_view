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


#include "object_manager.h"
#include <memory>
#include <mutex>
#include "utils.h"

namespace octo_flex {
void ObjectManager::submit(Object::Ptr obj, const std::string& layer_id) {
    if (obj == nullptr) return;
    obj->setInEditable();
    Layer::Ptr layer = findOrAddLayer(layer_id);
    layer->addObject(obj);
}

void ObjectManager::submitLayer(const std::vector<Object::Ptr>& objects, const std::string& layer_id) {
    for (auto& obj : objects) {
        if (obj != nullptr) {
            obj->setInEditable();
        }
    }
    Layer::Ptr layer = findOrAddLayer(layer_id);
    layer->setObjects(objects);
}

const std::pair<std::string, Object::Ptr> ObjectManager::findObject(const std::string& obj_id) {
    // Find object across all layers.
    std::unique_lock<std::mutex> lock(mtx_);
    for (auto& [layer_id, layer] : layers_) {
        Object::Ptr obj = layer->findObject(obj_id);
        if (obj != nullptr) {
            return std::make_pair(layer_id, obj);
        }
    }
    return std::make_pair("", nullptr);
}

const Layer::Ptr ObjectManager::findOrAddLayer(const std::string layer_id) {
    Layer::Ptr layer = nullptr;
    std::unique_lock<std::mutex> lock(mtx_);
    auto it = layers_.find(layer_id);
    if (it == layers_.end()) {
        layer = std::make_shared<Layer>(layer_id);
        layers_[layer_id] = layer;
    } else {
        layer = it->second;
    }
    return layer;
}

const Layer::Ptr ObjectManager::findLayer(const std::string layer_id) {
    Layer::Ptr layer = nullptr;

    std::unique_lock<std::mutex> lock(mtx_);
    auto it = layers_.find(layer_id);
    if (it != layers_.end()) {
        layer = it->second;
    }

    return layer;
}

const LayerList ObjectManager::layers() {
    LayerList layers;
    std::unique_lock<std::mutex> lock(mtx_);
    layers = layers_;
    return layers;
}

void ObjectManager::clearOutdatedObjects() {
    std::unique_lock<std::mutex> lock(mtx_);
    std::vector<Object::Ptr> all_outdated;

    // Collect outdated objects from all layers
    for (auto& [layer_id, layer] : layers_) {
        auto outdated = layer->collectOutdatedObjects();
        all_outdated.insert(all_outdated.end(), outdated.begin(), outdated.end());
    }

    // Release GPU resources for all outdated objects
    // This happens on the rendering thread with OpenGL context
    for (auto& obj : all_outdated) {
        if (obj) {
            for (auto& shape : obj->shapes()) {
                if (shape) {
                    shape->releaseResources();
                }
            }
        }
    }

    // Objects will be deleted when all_outdated goes out of scope
}

}  // namespace octo_flex
