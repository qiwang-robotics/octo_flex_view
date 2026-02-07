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


#ifndef DEF_H
#define DEF_H

#include <cmath>
#include <string>

namespace octo_flex {

typedef std::string Id;
typedef Id ObjectId;

struct Vec3 {
    Vec3();
    Vec3(double x, double y, double z);
    virtual ~Vec3();

    double x, y, z;

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(double scalar) const;
    Vec3 operator/(double scalar) const;

    double length() const;
    Vec3 normalized() const;
    double dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
};

struct Quaternion {
    Quaternion();
    Quaternion(double x, double y, double z, double w);
    void normalize();
    double x;
    double y;
    double z;
    double w;
};

}  // namespace octo_flex
#endif /* DEF_H */
