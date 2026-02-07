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


#ifndef OCTO_FLEX_CAMERA_H
#define OCTO_FLEX_CAMERA_H


#include <glm/glm.hpp>
#include <memory>

namespace octo_flex {

// Forward declaration
class CoordinateSystem;

class Camera {
   public:
    using Ptr = std::shared_ptr<Camera>;

    Camera();
    virtual ~Camera() = default;

    // Get view matrix.
    glm::mat4 getViewMatrix() const;

    // Get camera position.
    glm::vec3 getPosition() const;

    // Get camera forward direction.
    glm::vec3 getFront() const;

    // Get camera up direction.
    glm::vec3 getUp() const;

    // Get camera right direction.
    glm::vec3 getRight() const;

    // Get world up direction.
    glm::vec3 getWorldUp() const;

    // Set all camera vectors directly.
    void setVectors(const glm::vec3& front, const glm::vec3& up, const glm::vec3& right);

    // Rotate camera.
    void rotate(float deltaX, float deltaY);

    // Spherical rotation while keeping the ray-sphere intersection fixed.
    void rotateSphere(const glm::vec3& oldRayDir, const glm::vec3& newRayDir);

    // Move camera.
    void move(float deltaForward, float deltaRight, float deltaUp);

    // Move camera using a ray direction.
    void moveWithRay(float deltaForward, float deltaRight, float deltaUp, const glm::vec3& rayDirection);

    // Set camera position.
    void setPosition(const glm::vec3& position);

    // Set camera speed.
    void setSpeed(float speed);

    // Get camera speed.
    float getSpeed() const;

    // Get camera Euler angles.
    void getEulerAngles(float& yaw, float& pitch) const;

    // Update camera vectors.
    void updateCameraVectors();

    // Make camera look at a target.
    void lookAt(const glm::vec3& target);

    // Set the coordinate system for camera transformations.
    void setCoordinateSystem(std::shared_ptr<CoordinateSystem> coordSys);

    // Get the current coordinate system.
    std::shared_ptr<CoordinateSystem> getCoordinateSystem() const;

    // Enable or disable roll constraint (true = no roll constraint, false = constrain to coord sys up)
    void setRollEnabled(bool enabled);

    // Check if roll is enabled
    bool isRollEnabled() const;

   private:
    // Camera parameters.
    glm::vec3 position_;  // Camera position.
    glm::vec3 front_;     // Camera forward direction.
    glm::vec3 up_;        // Camera up direction.
    glm::vec3 right_;     // Camera right direction.
    glm::vec3 worldUp_;   // World up direction.

    // Euler angles.
    float yaw_;    // Yaw angle (around Y axis).
    float pitch_;  // Pitch angle (around X axis).

    // Camera options.
    float speed_;  // Camera movement speed.

    // Coordinate system for camera transformations.
    std::shared_ptr<CoordinateSystem> coordSys_;

    // Roll constraint flag (when false, camera up is constrained to coord sys up)
    bool rollEnabled_ = false;
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_CAMERA_H
