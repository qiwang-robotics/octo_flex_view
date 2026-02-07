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


#ifndef OBJECT_H
#define OBJECT_H

#include <vector>

#include <unordered_map>
#include "def.h"
#include "shape.h"

namespace octo_flex {
class Object {
   public:
    typedef std::shared_ptr<Object> Ptr;

   public:
    Object(ObjectId id);
    const ObjectId& id() const { return id_; }
    virtual ~Object(){};

    const std::string& info() const;
    void setInfo(const std::string& info, const Vec3 color = Vec3(0.2, 0.2, 0.2));
    Vec3 textColor() const;

    void merge(const Object::Ptr obj);

    Ptr clone();

    void addShape(Shape::Ptr shape);
    const std::vector<Shape::Ptr>& shapes() const;

    void move(const Vec3& vec);
    void rotate(const Quaternion& quad);

    bool isEditable() const;
    void setInEditable();

    void resetTransform();

    const Vec3& position() const;
    const Quaternion& orientation() const;

   private:
    Vec3 position_;
    Quaternion orientation_;

    bool editable_;
    ObjectId id_;
    std::string info_;
    std::string detail_;
    Vec3 text_color_;
    std::vector<Shape::Ptr> shapes_;
};

typedef std::unordered_map<std::string, Object::Ptr> ObjectList;
}  // namespace octo_flex

#endif /* OBJECT_H */
