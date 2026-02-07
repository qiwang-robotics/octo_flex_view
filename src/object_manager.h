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


#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <string>
#include "layer.h"

namespace octo_flex {
class ObjectManager {
   public:
    typedef std::shared_ptr<ObjectManager> Ptr;
    ObjectManager(){};
    virtual ~ObjectManager(){};
    const Layer::Ptr findLayer(const std::string layer_id);
    const Layer::Ptr findOrAddLayer(const std::string layer_id);

    const std::pair<std::string, Object::Ptr> findObject(const std::string& obj_id);
    const LayerList layers();

    void submit(Object::Ptr obj, const std::string& layer_id = "default");

    // Submit layer: replace all objects in the layer atomically
    void submitLayer(const std::vector<Object::Ptr>& objects, const std::string& layer_id = "default");

    // Clear outdated objects (call after rendering)
    void clearOutdatedObjects();

   private:
    LayerList layers_;
    std::mutex mtx_;
};
}  // namespace octo_flex

#endif /* OBJECT_MANAGER_H */
