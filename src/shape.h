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


#ifndef SHAPE_H
#define SHAPE_H

#include <memory>
#include <vector>

#include "def.h"

namespace octo_flex {
class Shape {
   public:
    enum ShapeType { Points = 0, Lines, Dash, Loop, Polygon, TexturedQuad };
    typedef std::shared_ptr<Shape> Ptr;

   public:
    Shape();
    virtual Ptr clone();
    Shape(ShapeType type, double width, double transparency);
    virtual ~Shape(){};

    void setPointsWithColor(const std::vector<Vec3>& points, const Vec3& color);
    void setPointsWithColor(const std::vector<Vec3>& points, const std::vector<Vec3>& colors);
    const std::vector<Vec3>& points() const;
    const std::vector<Vec3>& colors() const;
    const Vec3& color() const;
    const Vec3& color(size_t i) const;

    void setType(ShapeType type);
    ShapeType type() const;

    void setWidth(double width);
    double width() const;

    void setTransparency(double transparency);
    double transparency() const;

    void move(const Vec3& vec);
    void rotate(const Quaternion& quad);
    void scale(double sx, double sy, double sz);
    void scale(const Vec3& scale_factors);

    bool isEditable() const;
    void setInEditable();

    // Resource cleanup (called on main thread before deletion)
    virtual void releaseResources() {}

   private:
    bool editable_;

    ShapeType type_;
    double width_;
    double transparency_;
    std::vector<Vec3> points_;

    std::vector<Vec3> colors_;
    Vec3 color_;
};
}  // namespace octo_flex

#endif /* SHAPE_H */
