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


#include "camera.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include "coordinate_system.h"

namespace octo_flex {

Camera::Camera()
    : position_(0.0f, 0.0f, 5.0f),
      front_(0.0f, 0.0f, -1.0f),
      up_(0.0f, 0.0f, 1.0f),
      right_(1.0f, 0.0f, 0.0f),
      worldUp_(0.0f, 0.0f, 1.0f),
      yaw_(-90.0f),
      pitch_(0.0f),
      speed_(0.1f) {
    // Initialize camera vectors.
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const { return glm::lookAt(position_, position_ + front_, up_); }

glm::vec3 Camera::getPosition() const { return position_; }

glm::vec3 Camera::getFront() const { return front_; }

glm::vec3 Camera::getUp() const { return up_; }

glm::vec3 Camera::getRight() const { return right_; }

glm::vec3 Camera::getWorldUp() const { return worldUp_; }

void Camera::rotate(float deltaX, float deltaY) {
    // If a coordinate system is set, perform rotation in local space
    if (coordSys_) {
        // Transform camera front vector to local space
        glm::vec3 localFront = coordSys_->worldDirectionToLocal(front_);

        // Create rotation quaternion in local space
        // Z-up coordinate system: yaw rotates around Z axis, pitch rotates around local right
        float yawAngle = glm::radians(deltaX * 0.1f);
        float pitchAngle = glm::radians(deltaY * 0.1f);

        // Clamp pitch in local space to prevent flipping
        // In Z-up system, pitch is the angle from XY plane (Z component)
        float currentPitch = glm::degrees(asin(localFront.z));
        float newPitch = currentPitch + deltaY * 0.1f;
        newPitch = std::max(-89.0f, std::min(89.0f, newPitch));

        if (newPitch != currentPitch) {
            pitchAngle = glm::radians(newPitch - currentPitch);
        }

        // Calculate local right vector for pitch rotation
        glm::vec3 localRight = glm::normalize(glm::cross(localFront, glm::vec3(0.0f, 0.0f, 1.0f)));

        // Apply pitch (rotate around local right axis)
        glm::quat pitchRot = glm::angleAxis(pitchAngle, localRight);
        // Apply yaw (rotate around local Z axis, which is up)
        glm::quat yawRot = glm::angleAxis(yawAngle, glm::vec3(0.0f, 0.0f, 1.0f));

        // Combine rotations: pitch first, then yaw
        glm::quat rotation = yawRot * pitchRot;

        // Apply rotation ONLY to front vector (no roll)
        localFront = glm::normalize(rotation * localFront);

        // Transform front back to world space
        front_ = coordSys_->localDirectionToWorld(localFront);

        // If roll is enabled, rotate up vector as well (allow free roll)
        // Otherwise, reconstruct right and up to keep camera aligned with coord sys up
        if (rollEnabled_) {
            // Allow roll: rotate up vector with the same rotation
            glm::vec3 localUp = coordSys_->worldDirectionToLocal(up_);
            localUp = glm::normalize(rotation * localUp);
            up_ = coordSys_->localDirectionToWorld(localUp);
            right_ = glm::normalize(glm::cross(front_, up_));
        } else {
            // Get the coordinate system's up direction in world space (Z axis is up)
            glm::vec3 coordSysUp = coordSys_->localDirectionToWorld(glm::vec3(0.0f, 0.0f, 1.0f));

            // Reconstruct right and up vectors to keep camera aligned with coord sys up
            right_ = glm::normalize(glm::cross(front_, coordSysUp));
            up_ = glm::normalize(glm::cross(right_, front_));
        }

        // Derive Euler angles from the rotated forward vector (Z-up system)
        pitch_ = glm::degrees(asin(front_.z));
        yaw_ = glm::degrees(atan2(front_.x, -front_.y));
    } else {
        // Original behavior: update camera Euler angles directly
        yaw_ += deltaX * 0.1f;
        pitch_ += deltaY * 0.1f;

        // Clamp pitch to avoid flipping.
        pitch_ = std::max(-89.0f, std::min(89.0f, pitch_));

        // Update camera vectors.
        updateCameraVectors();
    }
}

void Camera::rotateSphere(const glm::vec3& oldRayDir, const glm::vec3& newRayDir) {
    // If a coordinate system is set, perform rotation in local space
    if (coordSys_) {
        // Transform ray directions to local space
        glm::vec3 v1 = glm::normalize(coordSys_->worldDirectionToLocal(oldRayDir));
        glm::vec3 v2 = glm::normalize(coordSys_->worldDirectionToLocal(newRayDir));

        // Compute rotation axis and angle in local space.
        glm::vec3 localRotationAxis = glm::cross(v2, v1);

        // If rotation axis is near zero, skip rotation.
        if (glm::length(localRotationAxis) < 0.0001f) {
            return;
        }

        localRotationAxis = glm::normalize(localRotationAxis);

        // Compute rotation angle (acos of dot product).
        float dotProduct = glm::dot(v1, v2);
        dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);  // Clamp to [-1, 1].
        float rotationAngle = acos(dotProduct);

        // Create rotation quaternion in local space.
        glm::quat rotation = glm::angleAxis(rotationAngle, localRotationAxis);

        // Transform camera front vector to local space
        glm::vec3 localFront = glm::normalize(coordSys_->worldDirectionToLocal(front_));

        // Apply rotation ONLY to front vector (prevents roll)
        localFront = glm::normalize(rotation * localFront);

        // Transform front back to world space
        front_ = coordSys_->localDirectionToWorld(localFront);

        // If roll is enabled, rotate up vector as well (allow free roll)
        // Otherwise, reconstruct right and up to keep camera aligned with coord sys up
        if (rollEnabled_) {
            // Allow roll: rotate up vector with the same rotation
            glm::vec3 localUp = coordSys_->worldDirectionToLocal(up_);
            localUp = glm::normalize(rotation * localUp);
            up_ = coordSys_->localDirectionToWorld(localUp);
            right_ = glm::normalize(glm::cross(front_, up_));
        } else {
            // Get the coordinate system's up direction in world space (Z axis is up)
            glm::vec3 coordSysUp = coordSys_->localDirectionToWorld(glm::vec3(0.0f, 0.0f, 1.0f));

            // Reconstruct right and up vectors to keep camera aligned with coord sys up
            right_ = glm::normalize(glm::cross(front_, coordSysUp));
            up_ = glm::normalize(glm::cross(right_, front_));
        }

        // Derive Euler angles from the rotated forward vector (Z-up system)
        pitch_ = glm::degrees(asin(front_.z));
        yaw_ = glm::degrees(atan2(front_.x, -front_.y));

        // Clamp pitch to avoid flipping.
        pitch_ = std::max(-89.0f, std::min(89.0f, pitch_));
    } else {
        // Original behavior: rotate in world space
        // Ensure direction vectors are normalized.
        glm::vec3 v1 = glm::normalize(oldRayDir);
        glm::vec3 v2 = glm::normalize(newRayDir);

        // Compute rotation axis and angle.
        glm::vec3 rotationAxis = glm::cross(v2, v1);

        // If rotation axis is near zero, skip rotation.
        if (glm::length(rotationAxis) < 0.0001f) {
            return;
        }

        rotationAxis = glm::normalize(rotationAxis);

        // Compute rotation angle (acos of dot product).
        float dotProduct = glm::dot(v1, v2);
        dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);  // Clamp to [-1, 1].
        float rotationAngle = acos(dotProduct);

        // Create rotation quaternion.
        glm::quat rotation = glm::angleAxis(rotationAngle, rotationAxis);

        // Apply rotation to camera forward vector.
        front_ = glm::normalize(rotation * front_);

        // Keep world-up unchanged; recompute right and up vectors.
        // This keeps camera up aligned with world Z axis.
        right_ = glm::normalize(glm::cross(front_, worldUp_));
        up_ = glm::normalize(glm::cross(right_, front_));

        // Derive Euler angles from the rotated forward vector.
        // Z-up system: forward (0,-1,0) corresponds to yaw=0, pitch=0
        pitch_ = glm::degrees(asin(front_.z));
        yaw_ = glm::degrees(atan2(front_.x, -front_.y));

        // Clamp pitch to avoid flipping.
        pitch_ = std::max(-89.0f, std::min(89.0f, pitch_));
    }
}

void Camera::move(float deltaForward, float deltaRight, float deltaUp) {
    // Compute movement vector.
    glm::vec3 moveVec(0.0f);

    // Forward/back movement.
    if (deltaForward != 0.0f) {
        moveVec += front_ * deltaForward;
    }

    // Left/right movement.
    if (deltaRight != 0.0f) {
        moveVec += right_ * deltaRight;
    }

    // Up/down movement.
    if (deltaUp != 0.0f) {
        moveVec += up_ * deltaUp;
    }

    // Update camera position if there is movement.
    if (glm::length(moveVec) > 0.0f) {
        // If a coordinate system is set, transform movement to local space
        if (coordSys_) {
            // Transform movement from world space to local space
            glm::vec3 localMoveVec = coordSys_->worldDirectionToLocal(moveVec);
            // Apply speed in local space
            localMoveVec *= speed_;
            // Transform back to world space and add to position
            position_ += coordSys_->localDirectionToWorld(localMoveVec);
        } else {
            // Original behavior: move in world space
            position_ += moveVec * speed_;
        }
    }
}

void Camera::moveWithRay(float deltaForward, float deltaRight, float deltaUp, const glm::vec3& rayDirection) {
    // Compute movement vector.
    glm::vec3 moveVec(0.0f);

    // Forward/back using the provided ray direction.
    if (deltaForward != 0.0f) {
        moveVec += rayDirection * deltaForward;
    }

    // Left/right movement.
    if (deltaRight != 0.0f) {
        moveVec += right_ * deltaRight;
    }

    // Up/down movement.
    if (deltaUp != 0.0f) {
        moveVec += up_ * deltaUp;
    }

    // Update camera position if there is movement.
    if (glm::length(moveVec) > 0.0f) {
        // If a coordinate system is set, transform movement to local space
        if (coordSys_) {
            // Transform movement from world space to local space
            glm::vec3 localMoveVec = coordSys_->worldDirectionToLocal(moveVec);
            // Apply speed in local space
            localMoveVec *= speed_;
            // Transform back to world space and add to position
            position_ += coordSys_->localDirectionToWorld(localMoveVec);
        } else {
            // Original behavior: move in world space
            position_ += moveVec * speed_;
        }
    }
}

void Camera::setPosition(const glm::vec3& position) { position_ = position; }

void Camera::setSpeed(float speed) { speed_ = speed; }

float Camera::getSpeed() const { return speed_; }

void Camera::getEulerAngles(float& yaw, float& pitch) const {
    yaw = yaw_;
    pitch = pitch_;
}

void Camera::updateCameraVectors() {
    // Compute camera forward vector.
    glm::vec3 front;
    front.x = -cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = -sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);

    // Compute camera right and up vectors.
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

void Camera::lookAt(const glm::vec3& target) {
    // Compute direction from camera to target.
    glm::vec3 direction = glm::normalize(target - position_);

    // Compute Euler angles.
    // Pitch is arcsin of the direction's Y component.
    pitch_ = glm::degrees(asin(direction.y));

    // Yaw is atan2 of direction X/Z.
    // Use -x and -z due to OpenGL camera coordinates.
    yaw_ = glm::degrees(atan2(-direction.x, -direction.z));

    // Clamp pitch to avoid flipping.
    pitch_ = std::max(-89.0f, std::min(89.0f, pitch_));

    // Update camera vectors.
    updateCameraVectors();
}

void Camera::setVectors(const glm::vec3& front, const glm::vec3& up, const glm::vec3& right) {
    // Set camera vectors directly without recomputing from Euler angles.
    front_ = glm::normalize(front);
    up_ = glm::normalize(up);
    right_ = glm::normalize(right);

    // Derive Euler angles from forward vector.
    pitch_ = glm::degrees(asin(front_.y));
    yaw_ = glm::degrees(atan2(-front_.x, -front_.z));

    // Clamp pitch.
    pitch_ = std::max(-89.0f, std::min(89.0f, pitch_));
}

void Camera::setCoordinateSystem(std::shared_ptr<CoordinateSystem> coordSys) { coordSys_ = coordSys; }

std::shared_ptr<CoordinateSystem> Camera::getCoordinateSystem() const { return coordSys_; }

void Camera::setRollEnabled(bool enabled) { rollEnabled_ = enabled; }

bool Camera::isRollEnabled() const { return rollEnabled_; }

}  // namespace octo_flex
