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


#include "def.h"

namespace octo_flex {

Vec3::Vec3() : x(0.0), y(0.0), z(0.0) {}
Vec3::Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
Vec3::~Vec3() {}

Vec3 Vec3::operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }

Vec3 Vec3::operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }

Vec3 Vec3::operator*(double scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }

Vec3 Vec3::operator/(double scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

double Vec3::length() const { return std::sqrt(x * x + y * y + z * z); }

Vec3 Vec3::normalized() const {
    double len = length();
    return Vec3(x / len, y / len, z / len);
}

double Vec3::dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

Quaternion::Quaternion() : x(0.0), y(0.0), z(0.0), w(1.0) {}
Quaternion::Quaternion(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
void Quaternion::normalize() {
    double magnitude = std::sqrt(x * x + y * y + z * z + w * w);

    if (magnitude < 1e-10) {
        x = 0;
        y = 0;
        z = 0;
        w = 1;
        return;
    }

    double invMagnitude = 1.0 / magnitude;
    x *= invMagnitude;
    y *= invMagnitude;
    z *= invMagnitude;
    w *= invMagnitude;
}

}  // namespace octo_flex
