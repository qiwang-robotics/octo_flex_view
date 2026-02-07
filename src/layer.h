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


#ifndef LAYER_H
#define LAYER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "object.h"

namespace octo_flex {
class Layer {
   public:
    typedef std::shared_ptr<Layer> Ptr;
    const std::string& id() const;

    Layer(std::string id) : id_(id) {}
    const std::string& id() { return id_; }

    const ObjectList objects();
    void addObject(Object::Ptr);
    void removeObject(std::string& object_id);
    void clear();

    // Replace all objects atomically (clear then add all)
    void setObjects(const std::vector<Object::Ptr>& objects);

    Object::Ptr findObject(std::string id);

    // Outdated objects management (for deferred deletion)
    std::vector<Object::Ptr> collectOutdatedObjects();

   private:
    ObjectList objects_;
    std::vector<Object::Ptr> outdated_objects_;  // Objects pending deletion
    std::string id_;
    std::mutex mtx_;
};
typedef std::unordered_map<std::string, Layer::Ptr> LayerList;
}  // namespace octo_flex

#endif /* LAYER_H */
