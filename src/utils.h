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


#ifndef UTILS_H
#define UTILS_H

#include "def.h"
#include "object.h"

namespace octo_flex {

// Generate a cube.
// Position: centered at origin (0,0,0).
// Orientation: edges aligned with X/Y/Z axes.
Object::Ptr generateCubic(const std::string& id, const Vec3& color, double width, double height, double length,
                          bool transparent = true);
// Generate a cylinder.
// Position: centered at origin (0,0,0).
// Orientation: Z axis is height; bottom at Z=-height/2, top at Z=height/2.
Object::Ptr generateCylinder(const std::string& id, const Vec3& color, double radius, double height,
                             bool transparent = true, bool simple_wire = false, int segments = 10);
// Generate a sphere.
// Position: centered at origin (0,0,0).
// Orientation: Y axis is the pole axis.
Object::Ptr generateSphere(const std::string& id, const Vec3& color, double radius, bool transparent = true,
                           bool simple_wire = false, int segments = 10);
// Generate a cone.
// Position: base center at origin (0,0,0), apex at (0,0,height).
// Orientation: positive Z is cone height direction.
Object::Ptr generateCone(const std::string& id, const Vec3& color, double radius, double height,
                         bool transparent = true, bool simple_wire = false, int segments = 10);
// Generate a pyramid.
// Position: base center at origin (0,0,0), apex at (0,0,height).
// Orientation: positive Z is pyramid height direction.
Object::Ptr generatePyramid(const std::string& id, const Vec3& color, double width, double height, double length,
                            bool transparent = true);
// Generate an arrow (3D: cylinder + pyramid).
// Position: overall center near origin.
// Orientation: arrow points along positive X.
Object::Ptr generateArrow(const std::string& id, const Vec3& color, double length, double shaft_radius,
                          double head_width, double head_length, bool transparent = true, int segments = 8);
// Generate a simple arrow (line-based with filled triangle head).
// Position: starts at origin.
// Orientation: arrow points along positive X.
Object::Ptr generateSimpleArrow(const std::string& id, const Vec3& color, double length, double head_width,
                                double head_length, double line_width = 2.0, bool transparent = false);
// Generate a quad.
// Position: centered at origin (0,0,0).
// Orientation: lies on the XY plane at Z=0.
Object::Ptr generateQuad(const std::string& id, const Vec3& color, double width, double length,
                         bool transparent = true);
// Generate an ellipsoid.
// Position: centered at origin (0,0,0).
// Orientation: Y axis is the pole axis.
// Scale: ellipsoid radii along X, Y, and Z axes.
Object::Ptr generateEllipsoid(const std::string& id, const Vec3& color, double radius_x, double radius_y,
                              double radius_z, bool transparent = true, bool simple_wire = false, int segments = 10);
// Generate a hemisphere.
// Position: centered at origin (0,0,0).
// Orientation: Y axis is the pole axis.
// Direction: top=true creates upper hemisphere (Y >= 0), top=false creates lower hemisphere (Y <= 0).
Object::Ptr generateHemisphere(const std::string& id, const Vec3& color, double radius, bool top = true,
                               bool transparent = true, bool simple_wire = false, int segments = 10);
// Generate a capsule.
// Position: centered at origin (0,0,0).
// Orientation: Z axis is height; composed of a cylinder with two complete spheres at ends.
// Note: Total height is height + 2 * radius (cylinder height + two spheres).
Object::Ptr generateCapsule(const std::string& id, const Vec3& color, double radius, double height,
                            bool transparent = true, int segments = 10);

Quaternion rotateX(double rad);
Quaternion rotateY(double rad);
Quaternion rotateZ(double rad);

// Quaternion-related calculations.
Quaternion quaternionMultiply(const Quaternion& q1, const Quaternion& q2);
Vec3 quaternionRotateVector(const Quaternion& q, const Vec3& v);
}  // namespace octo_flex

#endif /* UTILS_H */
