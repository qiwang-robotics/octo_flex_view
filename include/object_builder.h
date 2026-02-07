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

#ifndef OCTO_FLEX_OBJECT_BUILDER_H
#define OCTO_FLEX_OBJECT_BUILDER_H

#include <memory>
#include <string>
#include <vector>

#include "def.h"
#include "octo_flex_export.h"
#include "texture_image.h"

namespace octo_flex {

// Forward declarations
class Object;
class Shape;

/**
 * @brief Fluent builder for complex Object construction
 *
 * Provides chain-able methods for configuring objects with shapes,
 * transformations, textures, and metadata.
 *
 * @example Basic usage:
 * @code
 * auto obj = ObjectBuilder::begin("my_sphere")
 *     .sphere(Vec3(1, 0, 0), 1.0)
 *     .at(2, 0, 0)
 *     .rotateZ(M_PI / 4)
 *     .withInfo("Rotated red sphere")
 *     .build();
 * @endcode
 *
 * @example With texture:
 * @code
 * auto obj = ObjectBuilder::begin("textured")
 *     .texturedQuad(3.0, 3.0, "assets/texture.png", 0.8)
 *     .at(-2, 0, 0)
 *     .build();
 * @endcode
 */
class OCTO_FLEX_VIEW_API ObjectBuilder {
   public:
    /**
     * @brief Start building an object with the given ID
     * @param id Object identifier (supports hierarchical IDs with '#' separator)
     * @return ObjectBuilder instance for chaining
     */
    static ObjectBuilder begin(const std::string& id);

    // ========================================================================
    // Geometric Primitives
    // ========================================================================

    /**
     * @brief Add sphere shape
     * @param color RGB color (each component in [0, 1])
     * @param radius Sphere radius
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& sphere(const Vec3& color, double radius, bool transparent = true);

    /**
     * @brief Add box (cubic) shape
     * @param color RGB color (each component in [0, 1])
     * @param width Box width (X-axis)
     * @param height Box height (Z-axis)
     * @param depth Box depth (Y-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& box(const Vec3& color, double width, double height, double depth, bool transparent = true);

    /**
     * @brief Add cylinder shape
     * @param color RGB color (each component in [0, 1])
     * @param radius Cylinder radius
     * @param height Cylinder height (along Z-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& cylinder(const Vec3& color, double radius, double height, bool transparent = true);

    /**
     * @brief Add cone shape
     * @param color RGB color (each component in [0, 1])
     * @param radius Cone base radius
     * @param height Cone height (along Z-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& cone(const Vec3& color, double radius, double height, bool transparent = true);

    /**
     * @brief Add pyramid shape
     * @param color RGB color (each component in [0, 1])
     * @param width Pyramid width (X-axis)
     * @param height Pyramid height (Z-axis)
     * @param depth Pyramid depth (Y-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& pyramid(const Vec3& color, double width, double height, double depth, bool transparent = true);

    /**
     * @brief Add arrow shape (3D cylinder + pyramid)
     * @param color RGB color (each component in [0, 1])
     * @param length Arrow total length
     * @param shaft_radius Shaft radius (default: 0.05)
     * @param head_width Head cone width (default: 0.15)
     * @param head_length Head cone length (default: 0.3)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& arrow(const Vec3& color, double length, double shaft_radius = 0.05, double head_width = 0.15,
                         double head_length = 0.3, bool transparent = true);

    /**
     * @brief Add simplified arrow shape (line-based with filled triangle head)
     * @param color RGB color (each component in [0, 1])
     * @param length Arrow total length
     * @param head_width Head arrow width (default: 0.2 * length)
     * @param head_length Head arrow length (default: 0.3 * length)
     * @param line_width Line width in pixels (default: 2.0)
     * @param transparent Whether shape is transparent (default: false - opaque)
     * @return Reference to this builder (for chaining)
     *
     * @note This creates a simple line-based arrow (shaft + filled triangle arrowhead)
     *       pointing along the positive X-axis. Much lighter than the full 3D arrow.
     *       The triangle head is closed with both fill and outline.
     *
     * @example
     * @code
     * auto obj = ObjectBuilder::begin("simple_arrow")
     *     .simpleArrow(Vec3(1, 0, 0), 2.0)
     *     .at(0, 0, 0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& simpleArrow(const Vec3& color, double length, double head_width = -1.0, double head_length = -1.0,
                               double line_width = 2.0, bool transparent = false);

    /**
     * @brief Add quad (rectangle) shape
     * @param color RGB color (each component in [0, 1])
     * @param width Quad width (X-axis)
     * @param height Quad height (Y-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& quad(const Vec3& color, double width, double height, bool transparent = true);

    /**
     * @brief Add ellipsoid shape
     * @param color RGB color (each component in [0, 1])
     * @param radius_x Ellipsoid radius along X-axis
     * @param radius_y Ellipsoid radius along Y-axis
     * @param radius_z Ellipsoid radius along Z-axis
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& ellipsoid(const Vec3& color, double radius_x, double radius_y, double radius_z, bool transparent = true);

    /**
     * @brief Add capsule shape (cylinder with hemispherical caps)
     * @param color RGB color (each component in [0, 1])
     * @param radius Capsule radius
     * @param height Capsule total height (along Z-axis)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& capsule(const Vec3& color, double radius, double height, bool transparent = true);

    // ========================================================================
    // Basic Shapes (Points, Lines, Dash, Loop, Polygon)
    // ========================================================================

    /**
     * @brief Add points shape (renders vertices as points)
     * @param points Vertex positions
     * @param color Single color for all points
     * @param point_size Point size in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     *
     * @example
     * @code
     * std::vector<Vec3> pts = {{0, 0, 0}, {1, 1, 1}, {2, 0, 0}};
     * auto obj = ObjectBuilder::begin("points")
     *     .points(pts, Vec3(1, 0, 0), 3.0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& points(const std::vector<Vec3>& points, const Vec3& color, double point_size = 1.0,
                          bool transparent = true);

    /**
     * @brief Add points shape with per-vertex colors
     * @param points Vertex positions
     * @param colors Per-vertex colors (must match points size)
     * @param point_size Point size in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& points(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, double point_size = 1.0,
                          bool transparent = true);

    /**
     * @brief Add lines shape (renders pairs of vertices as line segments)
     * @param points Vertex positions (should be even number for paired lines)
     * @param color Single color for all lines
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     *
     * @note Points are interpreted as pairs: (p0,p1), (p2,p3), (p4,p5)...
     *
     * @example
     * @code
     * std::vector<Vec3> pts = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}};
     * auto obj = ObjectBuilder::begin("lines")
     *     .lines(pts, Vec3(0, 1, 0), 2.0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& lines(const std::vector<Vec3>& points, const Vec3& color, double line_width = 1.0,
                         bool transparent = true);

    /**
     * @brief Add lines shape with per-vertex colors
     * @param points Vertex positions (should be even number for paired lines)
     * @param colors Per-vertex colors (must match points size)
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& lines(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, double line_width = 1.0,
                         bool transparent = true);

    /**
     * @brief Add dashed lines shape (renders pairs of vertices as dashed line segments)
     * @param points Vertex positions (should be even number for paired lines)
     * @param color Single color for all lines
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     *
     * @note Points are interpreted as pairs: (p0,p1), (p2,p3), (p4,p5)...
     * @note Dash pattern is fixed (GL_LINE_STIPPLE with pattern 0x00FF)
     *
     * @example
     * @code
     * std::vector<Vec3> pts = {{0, 0, 0}, {2, 0, 0}};
     * auto obj = ObjectBuilder::begin("dashed")
     *     .dashedLines(pts, Vec3(1, 1, 0), 1.0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& dashedLines(const std::vector<Vec3>& points, const Vec3& color, double line_width = 1.0,
                               bool transparent = true);

    /**
     * @brief Add dashed lines shape with per-vertex colors
     * @param points Vertex positions (should be even number for paired lines)
     * @param colors Per-vertex colors (must match points size)
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& dashedLines(const std::vector<Vec3>& points, const std::vector<Vec3>& colors,
                               double line_width = 1.0, bool transparent = true);

    /**
     * @brief Add loop shape (renders vertices as a closed line loop)
     * @param points Vertex positions (automatically closes from last to first)
     * @param color Single color for the loop
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     *
     * @example
     * @code
     * std::vector<Vec3> pts = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
     * auto obj = ObjectBuilder::begin("loop")
     *     .loop(pts, Vec3(0, 0, 1), 2.0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& loop(const std::vector<Vec3>& points, const Vec3& color, double line_width = 1.0,
                        bool transparent = true);

    /**
     * @brief Add loop shape with per-vertex colors
     * @param points Vertex positions (automatically closes from last to first)
     * @param colors Per-vertex colors (must match points size)
     * @param line_width Line width in pixels (default: 1.0)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& loop(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, double line_width = 1.0,
                        bool transparent = true);

    /**
     * @brief Add polygon shape (renders vertices as a filled polygon)
     * @param points Vertex positions (defines polygon boundary)
     * @param color Single color for the polygon
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     *
     * @note For best results, vertices should be coplanar and in counter-clockwise order
     *
     * @example
     * @code
     * std::vector<Vec3> pts = {{0, 0, 0}, {1, 0, 0}, {0.5, 1, 0}};
     * auto obj = ObjectBuilder::begin("triangle")
     *     .polygon(pts, Vec3(1, 0, 1), true)
     *     .build();
     * @endcode
     */
    ObjectBuilder& polygon(const std::vector<Vec3>& points, const Vec3& color, bool transparent = true);

    /**
     * @brief Add polygon shape with per-vertex colors
     * @param points Vertex positions (defines polygon boundary)
     * @param colors Per-vertex colors (must match points size)
     * @param transparent Whether shape is transparent (default: true)
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& polygon(const std::vector<Vec3>& points, const std::vector<Vec3>& colors, bool transparent = true);

    // ========================================================================
    // Texture Support
    // ========================================================================

    /**
     * @brief Add textured quad from file (handles OpenGL context automatically)
     * @param width World-space width
     * @param height World-space height
     * @param texture_path Path to texture image file (PNG, JPG, BMP, etc.)
     * @param transparency Alpha value (0.0 = invisible, 1.0 = opaque, default: 1.0)
     * @return Reference to this builder (for chaining)
     *
     * @note Texture loading is handled automatically using lazy loading mechanism.
     *       No need to worry about OpenGL context timing.
     *
     * @example Loading from file:
     * @code
     * auto obj = ObjectBuilder::begin("textured")
     *     .texturedQuad(3.0, 3.0, "assets/texture.png", 0.8)
     *     .at(-2, 0, 0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& texturedQuad(double width, double height, const std::string& texture_path,
                                double transparency = 1.0);

    /**
     * @brief Add textured quad from custom image data
     * @param image Custom TextureImage with RGBA pixel data
     * @param width World-space width
     * @param height World-space height
     * @param transparency Alpha value (0.0 = invisible, 1.0 = opaque, default: 1.0)
     * @return Reference to this builder (for chaining)
     *
     * @note This allows programmatic texture generation (gradients, patterns, etc.)
     * @note Texture loading is handled automatically using lazy loading mechanism.
     *
     * @example Using custom image data:
     * @code
     * #include "textured_quad.h"
     *
     * // Create custom image
     * octo_flex::TextureImage image;
     * image.width = 256;
     * image.height = 256;
     * image.pixels.resize(256 * 256 * 4);
     *
     * // Fill with gradient
     * for (int y = 0; y < 256; y++) {
     *     for (int x = 0; x < 256; x++) {
     *         int i = (y * 256 + x) * 4;
     *         image.pixels[i + 0] = x;      // R
     *         image.pixels[i + 1] = y;      // G
     *         image.pixels[i + 2] = 0;      // B
     *         image.pixels[i + 3] = 255;    // A
     *     }
     * }
     *
     * // Use with ObjectBuilder
     * auto obj = ObjectBuilder::begin("gradient")
     *     .texturedQuad(image, 3.0, 3.0, 0.8)
     *     .at(-2, 0, 0)
     *     .build();
     * @endcode
     */
    ObjectBuilder& texturedQuad(const TextureImage& image, double width, double height, double transparency = 1.0);

    // ========================================================================
    // Custom Shapes
    // ========================================================================

    /**
     * @brief Add custom shape
     * @param shape Shape to add
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& addShape(std::shared_ptr<Shape> shape);

    // ========================================================================
    // Transformations
    // ========================================================================

    /**
     * @brief Set object position
     * @param position World position
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& at(const Vec3& position);

    /**
     * @brief Set object position
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& at(double x, double y, double z);

    /**
     * @brief Rotate around X-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& rotateX(double radians);

    /**
     * @brief Rotate around Y-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& rotateY(double radians);

    /**
     * @brief Rotate around Z-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& rotateZ(double radians);

    /**
     * @brief Apply quaternion rotation
     * @param quat Rotation quaternion
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& rotate(const Quaternion& quat);

    // ========================================================================
    // Metadata
    // ========================================================================

    /**
     * @brief Set object info text
     * @param info Information text
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& withInfo(const std::string& info);

    /**
     * @brief Set info text color
     * @param text_color RGB color for info text
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& withColor(const Vec3& text_color);

    // ========================================================================
    // Shape-Level Transformations (Phase 1)
    // ========================================================================

    /**
     * @brief Set local position for the next shape to be added
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return Reference to this builder (for chaining)
     *
     * @example
     * @code
     * auto obj = ObjectBuilder::begin("obj")
     *     .box(Vec3(1,0,0), 1, 1, 1)       // First box at origin
     *     .shapeAt(0, 0, 2)                 // Set position for next shape
     *     .sphere(Vec3(0,1,0), 0.5)        // This sphere at (0,0,2)
     *     .build();
     * @endcode
     */
    ObjectBuilder& shapeAt(double x, double y, double z);

    /**
     * @brief Set local position for the next shape to be added
     * @param position Local position relative to Object
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeAt(const Vec3& position);

    /**
     * @brief Rotate next shape around X-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeRotateX(double radians);

    /**
     * @brief Rotate next shape around Y-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeRotateY(double radians);

    /**
     * @brief Rotate next shape around Z-axis
     * @param radians Rotation angle in radians
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeRotateZ(double radians);

    /**
     * @brief Apply quaternion rotation to next shape
     * @param quat Rotation quaternion
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeRotate(const Quaternion& quat);

    /**
     * @brief Set non-uniform scale for next shape
     * @param sx X-axis scale
     * @param sy Y-axis scale
     * @param sz Z-axis scale
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeScale(double sx, double sy, double sz);

    /**
     * @brief Set uniform scale for next shape
     * @param uniform_scale Scale factor for all axes
     * @return Reference to this builder (for chaining)
     */
    ObjectBuilder& shapeScale(double uniform_scale);

    // ========================================================================
    // Build
    // ========================================================================

    /**
     * @brief Finalize and return the constructed object
     * @return Shared pointer to the built object
     *
     * @note After calling build(), this builder should not be reused.
     */
    std::shared_ptr<Object> build();

   private:
    std::shared_ptr<Object> object_;  // Object being built
    std::string info_text_;           // Pending info text
    Vec3 text_color_;                 // Pending text color
    bool has_text_color_ = false;     // Whether text color was set

    // Store texture data for deferred loading
    struct PendingTexture {
        std::string path;     // File path (if use_image == false)
        TextureImage image;   // Image data (if use_image == true)
        bool use_image;       // true = use image, false = use path
        double width;         // World-space width
        double height;        // World-space height
        double transparency;  // Alpha value
    };
    std::vector<PendingTexture> pending_textures_;

    // Store transforms for deferred shapes (texturedQuad)
    struct PendingTransform {
        enum Type { Move, Rotate } type;
        Vec3 move;
        Quaternion rotate;
    };
    std::vector<PendingTransform> pending_transforms_;

    // Phase 1: Store pending shape-level transforms
    Vec3 next_shape_position_{0, 0, 0};
    Quaternion next_shape_orientation_;  // Default unit quaternion
    Vec3 next_shape_scale_{1, 1, 1};

    // Private constructor (use begin() to create)
    explicit ObjectBuilder(const std::string& id);

    // Apply pending textures (called in build())
    void applyPendingTextures();

    // Phase 1: Apply and reset pending shape transforms
    void applyPendingShapeTransform(std::shared_ptr<Shape> shape);
    void resetPendingShapeTransform();
};

}  // namespace octo_flex

#endif  // OCTO_FLEX_OBJECT_BUILDER_H
