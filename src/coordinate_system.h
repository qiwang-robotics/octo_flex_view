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


#ifndef OCTO_FLEX_COORDINATE_SYSTEM_H
#define OCTO_FLEX_COORDINATE_SYSTEM_H


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>

namespace octo_flex {

// Type of coordinate system.
enum class CoordinateSystemType {
    Global,  // Global world coordinate system
    Local    // Local object coordinate system
};

/**
 * @brief CoordinateSystem class for defining coordinate reference frames.
 *
 * This class allows camera transformations to be locked to a specific
 * coordinate system, which can be either global (world) or local (object).
 * When a camera is locked to a local coordinate system, all transformations
 * are performed in the local space and then converted to world space.
 */
class CoordinateSystem {
   public:
    using Ptr = std::shared_ptr<CoordinateSystem>;

    /**
     * @brief Create a global coordinate system at origin with identity rotation.
     */
    static Ptr createGlobal();

    /**
     * @brief Create a local coordinate system at a specific position and orientation.
     * @param position Position in world space
     * @param orientation Orientation as a quaternion
     * @param objectId Optional object ID to track (for object-coordinate systems)
     */
    static Ptr createLocal(const glm::vec3& position, const glm::quat& orientation, const std::string& objectId = "");

    /**
     * @brief Create a coordinate system from look-at parameters.
     * @param eye Eye position
     * @param target Target position to look at
     * @param up Up direction
     * @param objectId Optional object ID
     */
    static Ptr createLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up,
                            const std::string& objectId = "");

    virtual ~CoordinateSystem() = default;

    // Get coordinate system type.
    CoordinateSystemType getType() const { return type_; }

    // Get position in world space.
    glm::vec3 getPosition() const { return position_; }

    // Get orientation as quaternion.
    glm::quat getOrientation() const { return orientation_; }

    // Get object ID (empty for global coordinate system).
    const std::string& getObjectId() const { return objectId_; }

    // Set position (for dynamic coordinate systems).
    void setPosition(const glm::vec3& position) { position_ = position; }

    // Set orientation (for dynamic coordinate systems).
    void setOrientation(const glm::quat& orientation) { orientation_ = orientation; }

    /**
     * @brief Transform a point from local space to world space.
     * @param localPoint Point in local coordinate system
     * @return Point in world coordinate system
     */
    glm::vec3 localToWorld(const glm::vec3& localPoint) const;

    /**
     * @brief Transform a point from world space to local space.
     * @param worldPoint Point in world coordinate system
     * @return Point in local coordinate system
     */
    glm::vec3 worldToLocal(const glm::vec3& worldPoint) const;

    /**
     * @brief Transform a direction from local space to world space.
     * Direction vectors are not affected by translation.
     * @param localDir Direction in local coordinate system
     * @return Direction in world coordinate system
     */
    glm::vec3 localDirectionToWorld(const glm::vec3& localDir) const;

    /**
     * @brief Transform a direction from world space to local space.
     * @param worldDir Direction in world coordinate system
     * @return Direction in local coordinate system
     */
    glm::vec3 worldDirectionToLocal(const glm::vec3& worldDir) const;

    /**
     * @brief Get the transformation matrix from local to world space.
     * @return 4x4 transformation matrix
     */
    glm::mat4 getLocalToWorldMatrix() const;

    /**
     * @brief Get the transformation matrix from world to local space.
     * @return 4x4 transformation matrix
     */
    glm::mat4 getWorldToLocalMatrix() const;

    /**
     * @brief Get the local X axis in world space.
     */
    glm::vec3 getLocalX() const;

    /**
     * @brief Get the local Y axis in world space.
     */
    glm::vec3 getLocalY() const;

    /**
     * @brief Get the local Z axis in world space.
     */
    glm::vec3 getLocalZ() const;

    /**
     * @brief Check if this coordinate system is valid.
     * Returns false for object coordinate systems when object is not found.
     */
    bool isValid() const { return valid_; }

    /**
     * @brief Invalidate the coordinate system (e.g., when object is deleted).
     */
    void invalidate() { valid_ = false; }

   private:
    // Private constructor - use static factory methods.
    CoordinateSystem(CoordinateSystemType type, const glm::vec3& position, const glm::quat& orientation,
                     const std::string& objectId);

    CoordinateSystemType type_;  // Type of coordinate system
    glm::vec3 position_;         // Origin in world space
    glm::quat orientation_;      // Orientation as quaternion
    std::string objectId_;       // Associated object ID (for local systems)
    bool valid_;                 // Validity flag
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_COORDINATE_SYSTEM_H
