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


#include "coordinate_system.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace octo_flex {

CoordinateSystem::CoordinateSystem(CoordinateSystemType type, const glm::vec3& position, const glm::quat& orientation,
                                   const std::string& objectId)
    : type_(type), position_(position), orientation_(orientation), objectId_(objectId), valid_(true) {}

CoordinateSystem::Ptr CoordinateSystem::createGlobal() {
    return std::shared_ptr<CoordinateSystem>(
        new CoordinateSystem(CoordinateSystemType::Global, glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::quat(1.0f, 0.0f, 0.0f, 0.0f),  // Identity quaternion
                             ""));
}

CoordinateSystem::Ptr CoordinateSystem::createLocal(const glm::vec3& position, const glm::quat& orientation,
                                                    const std::string& objectId) {
    return std::shared_ptr<CoordinateSystem>(
        new CoordinateSystem(CoordinateSystemType::Local, position, orientation, objectId));
}

CoordinateSystem::Ptr CoordinateSystem::createLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up,
                                                     const std::string& objectId) {
    // Create look-at view matrix
    glm::mat4 viewMatrix = glm::lookAt(eye, target, up);

    // Extract rotation from view matrix
    glm::quat orientation = glm::quat_cast(viewMatrix);

    // The orientation from view matrix is the inverse of what we need
    orientation = glm::inverse(orientation);

    return std::shared_ptr<CoordinateSystem>(
        new CoordinateSystem(CoordinateSystemType::Local, eye, orientation, objectId));
}

glm::vec3 CoordinateSystem::localToWorld(const glm::vec3& localPoint) const {
    // Apply rotation then translation
    return position_ + orientation_ * localPoint;
}

glm::vec3 CoordinateSystem::worldToLocal(const glm::vec3& worldPoint) const {
    // Apply inverse translation then inverse rotation
    return glm::inverse(orientation_) * (worldPoint - position_);
}

glm::vec3 CoordinateSystem::localDirectionToWorld(const glm::vec3& localDir) const {
    // Only apply rotation (directions are not affected by translation)
    return orientation_ * localDir;
}

glm::vec3 CoordinateSystem::worldDirectionToLocal(const glm::vec3& worldDir) const {
    // Only apply inverse rotation
    return glm::inverse(orientation_) * worldDir;
}

glm::mat4 CoordinateSystem::getLocalToWorldMatrix() const {
    // Build transformation matrix: T * R
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position_);
    glm::mat4 rotation = glm::mat4_cast(orientation_);
    return translation * rotation;
}

glm::mat4 CoordinateSystem::getWorldToLocalMatrix() const {
    // Inverse of local to world matrix
    return glm::inverse(getLocalToWorldMatrix());
}

glm::vec3 CoordinateSystem::getLocalX() const { return localDirectionToWorld(glm::vec3(1.0f, 0.0f, 0.0f)); }

glm::vec3 CoordinateSystem::getLocalY() const { return localDirectionToWorld(glm::vec3(0.0f, 1.0f, 0.0f)); }

glm::vec3 CoordinateSystem::getLocalZ() const { return localDirectionToWorld(glm::vec3(0.0f, 0.0f, 1.0f)); }

}  // namespace octo_flex
