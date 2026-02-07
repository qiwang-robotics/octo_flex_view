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


#include "utils.h"
#include <cmath>
#include "def.h"

namespace octo_flex {
Object::Ptr generateCubic(const std::string& id, const Vec3& color, double length, double width, double height,
                          bool transparent) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr cubic_obj = std::make_shared<Object>(id);
    cubic_obj->setInfo(cubic_obj->id(), text_color);

    // Precompute half lengths.
    double half_length = length / 2;
    double half_width = width / 2;
    double half_height = height / 2;

    // Define the 8 cube vertices.
    std::vector<Vec3> vertices = {
        Vec3(-half_length, -half_width, -half_height), Vec3(half_length, -half_width, -half_height),
        Vec3(half_length, half_width, -half_height),   Vec3(-half_length, half_width, -half_height),
        Vec3(-half_length, -half_width, half_height),  Vec3(half_length, -half_width, half_height),
        Vec3(half_length, half_width, half_height),    Vec3(-half_length, half_width, half_height)};

    // Define the 12 edges of the cube
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom face
        {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Vertical edges
    };

    // Create a shape for each edge
    for (const auto& edge : edges) {
        Shape::Ptr edge_shape = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> points = {vertices[edge.first], vertices[edge.second]};
        edge_shape->setPointsWithColor(points, edge_color);  // Use the provided color.
        cubic_obj->addShape(edge_shape);
    }

    // Define the 6 faces of the cube
    std::vector<std::vector<int>> faces = {
        {0, 1, 2, 3},  // Bottom face
        {4, 5, 6, 7},  // Top face
        {0, 1, 5, 4},  // Front face
        {2, 3, 7, 6},  // Back face
        {0, 3, 7, 4},  // Left face
        {1, 2, 6, 5}   // Right face
    };

    // Create translucent faces and use Loop for closed outlines.
    for (size_t i = 0; i < faces.size(); ++i) {
        // Create translucent polygon.
        Shape::Ptr face_shape = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);  // Increase transparency.

        std::vector<Vec3> face_points;
        for (int idx : faces[i]) {
            face_points.push_back(vertices[idx]);
        }

        // Use a slightly lighter color.
        face_shape->setPointsWithColor(face_points, face_color);
        cubic_obj->addShape(face_shape);

        // Add closed outline so polygon edges are clear.
        Shape::Ptr outline_shape = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        outline_shape->setPointsWithColor(face_points, edge_color);
        cubic_obj->addShape(outline_shape);
    }

    return cubic_obj;
}

Object::Ptr generateCylinder(const std::string& id, const Vec3& color, double radius, double height, bool transparent,
                             bool simple_wire, int segments) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr cylinder_obj = std::make_shared<Object>(id);
    cylinder_obj->setInfo(cylinder_obj->id(), text_color);

    // Generate top and bottom circle vertices.
    std::vector<Vec3> top_vertices;
    std::vector<Vec3> bottom_vertices;
    Vec3 top_center(0, 0, height / 2);      // Top center on Z axis.
    Vec3 bottom_center(0, 0, -height / 2);  // Bottom center on Z axis.

    for (int i = 0; i < segments; ++i) {
        double angle = 2 * M_PI * i / segments;
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);                     // Use Y axis.
        top_vertices.push_back(Vec3(x, y, top_center.z));        // Use updated top center.
        bottom_vertices.push_back(Vec3(x, y, bottom_center.z));  // Use updated bottom center.
    }

    // Create top and bottom circle edges.
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        // Bottom circle edge.
        Shape::Ptr bottom_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> bottom_edge_points = {bottom_vertices[i], bottom_vertices[next]};
        bottom_edge->setPointsWithColor(bottom_edge_points, edge_color);
        cylinder_obj->addShape(bottom_edge);

        // Top circle edge.
        Shape::Ptr top_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> top_edge_points = {top_vertices[i], top_vertices[next]};
        top_edge->setPointsWithColor(top_edge_points, edge_color);
        cylinder_obj->addShape(top_edge);

        if (simple_wire == false) continue;
        // Connect top and bottom edges.
        Shape::Ptr side_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> side_edge_points = {bottom_vertices[i], top_vertices[i]};
        side_edge->setPointsWithColor(side_edge_points, edge_color);
        cylinder_obj->addShape(side_edge);
    }

    if (simple_wire == false) {
        // Draw two vertical edges at symmetric positions.
        Shape::Ptr vertical_edge1 = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> vertical_edge_points1 = {top_vertices[0], bottom_vertices[0]};
        vertical_edge1->setPointsWithColor(vertical_edge_points1, edge_color);
        cylinder_obj->addShape(vertical_edge1);

        Shape::Ptr vertical_edge2 = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> vertical_edge_points2 = {top_vertices[segments / 2], bottom_vertices[segments / 2]};
        vertical_edge2->setPointsWithColor(vertical_edge_points2, edge_color);
        cylinder_obj->addShape(vertical_edge2);
    }

    // Create top and bottom faces.
    Shape::Ptr top_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    top_face->setPointsWithColor(top_vertices, face_color);
    cylinder_obj->addShape(top_face);

    Shape::Ptr top_loop = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
    top_loop->setPointsWithColor(top_vertices, edge_color);
    cylinder_obj->addShape(top_loop);

    Shape::Ptr bottom_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    bottom_face->setPointsWithColor(bottom_vertices, face_color);
    cylinder_obj->addShape(bottom_face);

    Shape::Ptr bottom_loop = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
    bottom_loop->setPointsWithColor(bottom_vertices, edge_color);
    cylinder_obj->addShape(bottom_loop);

    // Create side faces.
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        std::vector<Vec3> side_face_points = {bottom_vertices[i], bottom_vertices[next], top_vertices[next],
                                              top_vertices[i]};

        Shape::Ptr side_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
        side_face->setPointsWithColor(side_face_points, face_color);
        cylinder_obj->addShape(side_face);
    }

    return cylinder_obj;
}

Object::Ptr generateSphere(const std::string& id, const Vec3& color, double radius, bool transparent, bool simple_wire,
                           int segments) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr sphere_obj = std::make_shared<Object>(id);
    sphere_obj->setInfo(sphere_obj->id(), text_color);

    // Generate sphere vertices.
    std::vector<std::vector<Vec3>> vertices;

    // Horizontal segments (latitude).
    for (int i = 0; i <= segments; ++i) {
        double phi = M_PI * i / segments;  // From 0 to pi.
        std::vector<Vec3> circle;

        // Vertical segments (longitude).
        for (int j = 0; j < segments; ++j) {
            double theta = 2 * M_PI * j / segments;  // From 0 to 2pi.
            double x = radius * sin(phi) * cos(theta);
            double y = radius * cos(phi);
            double z = radius * sin(phi) * sin(theta);
            circle.push_back(Vec3(x, y, z));
        }
        vertices.push_back(circle);
    }

    if (simple_wire) {
        // Create meridians (vertical lines).
        for (int j = 0; j < segments; ++j) {
            for (int i = 0; i < segments; ++i) {
                Shape::Ptr line = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
                std::vector<Vec3> line_points = {vertices[i][j], vertices[i + 1][j]};
                line->setPointsWithColor(line_points, edge_color);
                sphere_obj->addShape(line);
            }
        }

        // Create parallels (horizontal lines).
        for (int i = 1; i < segments; ++i) {  // Skip top and bottom points (poles).
            for (int j = 0; j < segments; ++j) {
                int next = (j + 1) % segments;
                Shape::Ptr line = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
                std::vector<Vec3> line_points = {vertices[i][j], vertices[i][next]};
                line->setPointsWithColor(line_points, edge_color);
                sphere_obj->addShape(line);
            }
        }
    } else {
        Shape::Ptr line1 = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        Shape::Ptr line3 = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        std::vector<Vec3> line_points1, line_points3;
        int segments_2 = segments * 2;
        for (int i = 0; i <= segments_2; ++i) {
            double theta = 2 * M_PI * i / (segments_2);  // Longitude angle.
            double x = radius * cos(theta);
            double y = radius * sin(theta);
            line_points1.emplace_back(x, y, 0);
            line_points3.emplace_back(0, x, y);
        }
        line1->setPointsWithColor(line_points1, edge_color);
        line3->setPointsWithColor(line_points3, edge_color);
        sphere_obj->addShape(line1);
        sphere_obj->addShape(line3);

        Shape::Ptr line2 = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        std::vector<Vec3> line_points2;
        for (int i = 0; i <= segments; ++i) {
            double theta = 2 * M_PI * i / segments;  // Longitude angle.
            double x = radius * cos(theta);
            double y = radius * sin(theta);
            line_points2.emplace_back(x, 0, y);
        }
        line2->setPointsWithColor(line_points2, edge_color);
        sphere_obj->addShape(line2);
    }

    // Create faces.
    // Create cap triangles.
    for (int j = 0; j < segments; ++j) {
        int next_j = (j + 1) % segments;

        // Create bottom cap triangle.
        std::vector<Vec3> base_face_points = {vertices[segments - 1][j], vertices[segments - 1][next_j],
                                              Vec3(0, -radius, 0)};
        Shape::Ptr base_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
        base_face->setPointsWithColor(base_face_points, face_color);
        sphere_obj->addShape(base_face);
    }

    // Create quad faces (excluding pole triangles).
    for (int i = 0; i < segments - 1; ++i) {
        for (int j = 0; j < segments; ++j) {
            int next_j = (j + 1) % segments;

            std::vector<Vec3> face_points = {vertices[i][j], vertices[i][next_j], vertices[i + 1][next_j],
                                             vertices[i + 1][j]};
            Shape::Ptr face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
            face->setPointsWithColor(face_points, face_color);
            sphere_obj->addShape(face);
        }
    }

    return sphere_obj;
}

Object::Ptr generateHemisphere(const std::string& id, const Vec3& color, double radius, bool top, bool transparent,
                               bool simple_wire, int segments) {
    // Setup colors and transparency.
    float transparency = transparent ? 0.1 : 1.0;
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr hemisphere_obj = std::make_shared<Object>(id);
    hemisphere_obj->setInfo(hemisphere_obj->id(), text_color);

    // Y-axis sign: +1 for top hemisphere (Y>=0), -1 for bottom hemisphere (Y<=0).
    double y_sign = top ? 1.0 : -1.0;

    // Pole position for this hemisphere.
    Vec3 pole(0, y_sign * radius, 0);

    // Generate hemisphere vertices.
    // Latitude rings from pole to equator: phi from 0 to pi/2 for top, or pi/2 to pi for bottom.
    int latitude_segments = segments / 2;
    double phi_start = top ? 0.0 : M_PI / 2;
    double phi_range = M_PI / 2;

    std::vector<std::vector<Vec3>> vertices;
    for (int i = 0; i <= latitude_segments; ++i) {
        double phi = phi_start + phi_range * i / latitude_segments;
        std::vector<Vec3> circle;

        for (int j = 0; j < segments; ++j) {
            double theta = 2 * M_PI * j / segments;
            double x = radius * sin(phi) * cos(theta);
            double y = radius * cos(phi);
            double z = radius * sin(phi) * sin(theta);
            circle.push_back(Vec3(x, y, z));
        }
        vertices.push_back(circle);
    }

    // Generate wireframe.
    if (simple_wire) {
        // Full wireframe: meridians and parallels.
        for (int j = 0; j < segments; ++j) {
            for (int i = 0; i < latitude_segments; ++i) {
                Shape::Ptr meridian = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
                meridian->setPointsWithColor({vertices[i][j], vertices[i + 1][j]}, edge_color);
                hemisphere_obj->addShape(meridian);
            }
        }

        for (int i = 1; i < latitude_segments; ++i) {
            for (int j = 0; j < segments; ++j) {
                int next = (j + 1) % segments;
                Shape::Ptr parallel = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
                parallel->setPointsWithColor({vertices[i][j], vertices[i][next]}, edge_color);
                hemisphere_obj->addShape(parallel);
            }
        }
    } else {
        // Simple wireframe: two semi-circles and one equator circle.
        int wireframe_segments = segments * 2;
        std::vector<Vec3> semicircle_xz, semicircle_yz;

        // Generate two perpendicular semi-circles through the pole.
        for (int i = 0; i < wireframe_segments; ++i) {
            double theta = M_PI * i / wireframe_segments;
            double x = radius * cos(theta);
            double y = radius * sin(theta) * y_sign;  // Apply hemisphere direction.

            semicircle_xz.emplace_back(x, y, 0);  // Semi-circle in XY plane at Z=0.
            semicircle_xz.emplace_back(radius * cos(M_PI * (i + 1) / wireframe_segments),
                                       radius * sin(M_PI * (i + 1) / wireframe_segments) * y_sign, 0);

            semicircle_yz.emplace_back(0, y, x);  // Semi-circle in YZ plane at X=0.
            semicircle_yz.emplace_back(0, radius * sin(M_PI * (i + 1) / wireframe_segments) * y_sign,
                                       radius * cos(M_PI * (i + 1) / wireframe_segments));
        }

        Shape::Ptr line_xz = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        Shape::Ptr line_yz = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        line_xz->setPointsWithColor(semicircle_xz, edge_color);
        line_yz->setPointsWithColor(semicircle_yz, edge_color);
        hemisphere_obj->addShape(line_xz);
        hemisphere_obj->addShape(line_yz);

        // Equator circle at Y=0.
        Shape::Ptr equator = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        std::vector<Vec3> equator_points;
        for (int i = 0; i <= segments; ++i) {
            double theta = 2 * M_PI * i / segments;
            equator_points.emplace_back(radius * cos(theta), 0, radius * sin(theta));
        }
        equator->setPointsWithColor(equator_points, edge_color);
        hemisphere_obj->addShape(equator);
    }

    // Generate faces.
    // Pole triangles.
    int pole_ring_index = top ? 0 : latitude_segments - 1;
    for (int j = 0; j < segments; ++j) {
        int next_j = (j + 1) % segments;
        std::vector<Vec3> pole_triangle = {vertices[pole_ring_index][j], vertices[pole_ring_index][next_j], pole};

        Shape::Ptr pole_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
        pole_face->setPointsWithColor(pole_triangle, face_color);
        hemisphere_obj->addShape(pole_face);
    }

    // Quad faces between latitude rings.
    for (int i = 0; i < latitude_segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int next_j = (j + 1) % segments;
            std::vector<Vec3> quad = {vertices[i][j], vertices[i][next_j], vertices[i + 1][next_j], vertices[i + 1][j]};

            Shape::Ptr face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
            face->setPointsWithColor(quad, face_color);
            hemisphere_obj->addShape(face);
        }
    }

    return hemisphere_obj;
}

Object::Ptr generateCone(const std::string& id, const Vec3& color, double radius, double height, bool transparent,
                         bool simple_wire, int segments) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr cone_obj = std::make_shared<Object>(id);
    cone_obj->setInfo(cone_obj->id(), text_color);

    // Generate vertices.
    Vec3 top(0, 0, height);           // Cone apex.
    std::vector<Vec3> base_vertices;  // Base vertices.

    for (int i = 0; i < segments; ++i) {
        double angle = 2 * M_PI * i / segments;
        double x = radius * std::cos(angle);
        double z = radius * std::sin(angle);
        base_vertices.push_back(Vec3(x, z, 0));
    }

    // Create base edges.
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;

        // Base edge.
        Shape::Ptr base_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> base_edge_points = {base_vertices[i], base_vertices[next]};
        base_edge->setPointsWithColor(base_edge_points, edge_color);
        cone_obj->addShape(base_edge);

        if (simple_wire == false) continue;
        // Edges from apex to base.
        Shape::Ptr side_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> side_edge_points = {top, base_vertices[i]};
        side_edge->setPointsWithColor(side_edge_points, edge_color);
        cone_obj->addShape(side_edge);
    }

    if (simple_wire == false) {
        // Draw two apex-to-base lines.
        Shape::Ptr top_to_base1 = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> line1_points = {top, Vec3(-radius, 0, 0)};
        top_to_base1->setPointsWithColor(line1_points, edge_color);
        cone_obj->addShape(top_to_base1);

        Shape::Ptr top_to_base2 = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> line2_points = {top, Vec3(radius, 0, 0)};
        top_to_base2->setPointsWithColor(line2_points, edge_color);
        cone_obj->addShape(top_to_base2);
    }

    // Create base face.
    Shape::Ptr base_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    base_face->setPointsWithColor(base_vertices, face_color);
    cone_obj->addShape(base_face);

    Shape::Ptr base_loop = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
    base_loop->setPointsWithColor(base_vertices, edge_color);
    cone_obj->addShape(base_loop);

    // Create side faces (triangles).
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        std::vector<Vec3> side_face_points = {base_vertices[i], base_vertices[next], top};

        Shape::Ptr side_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
        side_face->setPointsWithColor(side_face_points, face_color);
        cone_obj->addShape(side_face);
    }

    return cone_obj;
}

Object::Ptr generatePyramid(const std::string& id, const Vec3& color, double width, double height, double length,
                            bool transparent) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    Object::Ptr pyramid_obj = std::make_shared<Object>(id);
    pyramid_obj->setInfo(pyramid_obj->id(), text_color);

    // Generate vertices.
    Vec3 top(0, 0, height);  // Pyramid apex.

    // Four base vertices.
    std::vector<Vec3> base_vertices = {
        Vec3(-width / 2, -length / 2, 0),  // Front-left.
        Vec3(width / 2, -length / 2, 0),   // Front-right.
        Vec3(width / 2, length / 2, 0),    // Back-right.
        Vec3(-width / 2, length / 2, 0)    // Back-left.
    };

    // Create base edges.
    for (int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;

        // Base edge.
        Shape::Ptr base_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> base_edge_points = {base_vertices[i], base_vertices[next]};
        base_edge->setPointsWithColor(base_edge_points, edge_color);
        pyramid_obj->addShape(base_edge);

        // Edge from apex to base vertex.
        Shape::Ptr side_edge = std::make_shared<Shape>(Shape::Lines, 1.0, 1.0);
        std::vector<Vec3> side_edge_points = {top, base_vertices[i]};
        side_edge->setPointsWithColor(side_edge_points, edge_color);
        pyramid_obj->addShape(side_edge);
    }

    // Create base face.
    Shape::Ptr base_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    base_face->setPointsWithColor(base_vertices, face_color);
    pyramid_obj->addShape(base_face);

    Shape::Ptr base_loop = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
    base_loop->setPointsWithColor(base_vertices, edge_color);
    pyramid_obj->addShape(base_loop);

    // Create side faces (four triangles).
    for (int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;
        std::vector<Vec3> side_face_points = {base_vertices[i], base_vertices[next], top};

        Shape::Ptr side_face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
        side_face->setPointsWithColor(side_face_points, face_color);
        pyramid_obj->addShape(side_face);

        Shape::Ptr side_loop = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
        side_loop->setPointsWithColor(side_face_points, edge_color);
        pyramid_obj->addShape(side_loop);
    }

    return pyramid_obj;
}

Object::Ptr generateArrow(const std::string& id, const Vec3& color, double length, double shaft_radius,
                          double head_width, double head_length, bool transparent, int segments) {
    double shaft_length = length - head_length;
    // Create the main arrow object.
    Object::Ptr arrow_obj = std::make_shared<Object>(id);

    // Generate shaft (cylinder).
    std::string shaft_id = id + "_shaft";
    Object::Ptr shaft = generateCylinder(shaft_id, color, shaft_radius, shaft_length, transparent, false, segments);

    // Generate arrowhead (pyramid).
    std::string head_id = id + "_head";
    Object::Ptr head = generatePyramid(head_id, color, head_width, head_length, head_width, transparent);
    // Object::Ptr head = generateCone(head_id, color, head_width / 2, head_length, transparent);

    // Move pyramid to the top of the cylinder.
    shaft->move(Vec3(0, 0, shaft_length / 2));
    head->move(Vec3(0, 0, shaft_length));

    // Merge shaft and head.
    arrow_obj->merge(shaft);
    arrow_obj->merge(head);
    arrow_obj->rotate(rotateY(M_PI_2));

    // Set arrow info text.
    Vec3 text_color = color * 0.75;
    arrow_obj->setInfo(arrow_obj->id(), text_color);

    return arrow_obj;
}

Object::Ptr generateSimpleArrow(const std::string& id, const Vec3& color, double length, double head_width,
                                double head_length, double line_width, bool transparent) {
    float transparency = 1.0;
    if (transparent) {
        transparency = 0.1;
    }

    // Create the arrow object
    Object::Ptr arrow_obj = std::make_shared<Object>(id);
    Vec3 text_color = color * 0.75;
    arrow_obj->setInfo(arrow_obj->id(), text_color);

    // Calculate shaft length
    double shaft_length = length - head_length;

    // Create arrow shaft (main line along X-axis)
    Shape::Ptr shaft = std::make_shared<Shape>(Shape::Lines, line_width, 1.0);
    std::vector<Vec3> shaft_points = {Vec3(0, 0, 0), Vec3(shaft_length, 0, 0)};
    shaft->setPointsWithColor(shaft_points, color);
    arrow_obj->addShape(shaft);

    // Create arrowhead as a closed triangle (loop)
    Vec3 arrow_tip = Vec3(length, 0, 0);
    Vec3 left_base = Vec3(shaft_length, head_width / 2, 0);
    Vec3 right_base = Vec3(shaft_length, -head_width / 2, 0);

    std::vector<Vec3> triangle_points = {arrow_tip, left_base, right_base};

    // Filled triangle
    Shape::Ptr triangle_fill = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    triangle_fill->setPointsWithColor(triangle_points, color);
    arrow_obj->addShape(triangle_fill);

    // Triangle outline (always opaque)
    Shape::Ptr triangle_outline = std::make_shared<Shape>(Shape::Loop, line_width, 1.0);
    triangle_outline->setPointsWithColor(triangle_points, color);
    arrow_obj->addShape(triangle_outline);

    return arrow_obj;
}

Quaternion rotateX(double rad) {
    double half_angle = rad * 0.5;
    return Quaternion(std::sin(half_angle), 0.0, 0.0, std::cos(half_angle));
}

Quaternion rotateY(double rad) {
    double half_angle = rad * 0.5;
    return Quaternion(0.0, std::sin(half_angle), 0.0, std::cos(half_angle));
}

Quaternion rotateZ(double rad) {
    double half_angle = rad * 0.5;
    return Quaternion(0.0, 0.0, std::sin(half_angle), std::cos(half_angle));
}

Object::Ptr generateQuad(const std::string& id, const Vec3& color, double width, double length, bool transparent) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    // Create object with an ID that does not include the layer ID.
    Object::Ptr quad_obj = std::make_shared<Object>(id);
    quad_obj->setInfo(quad_obj->id(), text_color);

    // Compute the four vertices.
    double half_width = width / 2;
    double half_length = length / 2;  // Use length.

    std::vector<Vec3> vertices = {
        Vec3(-half_width, -half_length, 0),  // Bottom-left.
        Vec3(half_width, -half_length, 0),   // Bottom-right.
        Vec3(half_width, half_length, 0),    // Top-right.
        Vec3(-half_width, half_length, 0)    // Top-left.
    };

    // Create quad face.
    Shape::Ptr face = std::make_shared<Shape>(Shape::Polygon, 1.0, transparency);
    face->setPointsWithColor(vertices, face_color);
    quad_obj->addShape(face);

    // Create outline.
    Shape::Ptr outline = std::make_shared<Shape>(Shape::Loop, 1.0, 1.0);
    outline->setPointsWithColor(vertices, edge_color);
    quad_obj->addShape(outline);

    return quad_obj;
}

Object::Ptr generateEllipsoid(const std::string& id, const Vec3& color, double radius_x, double radius_y,
                              double radius_z, bool transparent, bool simple_wire, int segments) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 face_color = color * 0.9;
    Vec3 edge_color = color * 0.5;
    Vec3 text_color = color * 0.75;

    // Generate a sphere with radius 1.0
    Object::Ptr sphere_obj = generateSphere(id, color, 1.0, transparent, simple_wire, segments);

    // Scale the sphere to create an ellipsoid with the specified radii
    for (auto& shape : sphere_obj->shapes()) {
        shape->scale(radius_x, radius_y, radius_z);
    }

    // Update the info text
    sphere_obj->setInfo(sphere_obj->id(), text_color);

    return sphere_obj;
}

// Quaternion helper implementations.
Quaternion quaternionMultiply(const Quaternion& q1, const Quaternion& q2) {
    Quaternion result;

    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    return result;
}

Quaternion quaternionConjugate(const Quaternion& q) { return Quaternion(-q.x, -q.y, -q.z, q.w); }

Vec3 quaternionRotateVector(const Quaternion& q, const Vec3& v) {
    // Convert vector to quaternion form (real part is 0).
    Quaternion vQuaternion(v.x, v.y, v.z, 0.0);

    // Rotate using v' = q * v * q^(-1).
    // q^(-1) is the conjugate (assuming q is a unit quaternion).
    Quaternion qConjugate = quaternionConjugate(q);

    // Compute q * v.
    Quaternion temp = quaternionMultiply(q, vQuaternion);

    // Compute (q * v) * q^(-1).
    Quaternion rotatedQuaternion = quaternionMultiply(temp, qConjugate);

    // Extract vector part from rotated quaternion.
    return Vec3(rotatedQuaternion.x, rotatedQuaternion.y, rotatedQuaternion.z);
}

Object::Ptr generateCapsule(const std::string& id, const Vec3& color, double radius, double height, bool transparent,
                            int segments) {
    float transparency = 1;
    if (transparent) {
        transparency = 0.1;
    }
    Vec3 text_color = color * 0.75;

    // Create the main capsule object.
    Object::Ptr capsule_obj = std::make_shared<Object>(id);
    capsule_obj->setInfo(capsule_obj->id(), text_color);

    // The cylinder height is the total height minus the two hemispherical caps (2 * radius).
    double cylinder_height = height;  // - 2.0 * radius;

    // Generate the middle cylinder (only if height > 2 * radius).
    // if (cylinder_height > 2.0 * radius) {
    std::string cylinder_id = id + "_cylinder";
    Object::Ptr cylinder = generateCylinder(cylinder_id, color, radius, cylinder_height, transparent, false, segments);
    capsule_obj->merge(cylinder);
    //}

    // Generate the two hemispherical caps.
    // Top hemisphere: use bottom hemisphere (Y<=0) so pole points to +Z after rotation.
    std::string top_hemi_id = id + "_top_hemisphere";
    Object::Ptr top_hemi = generateHemisphere(top_hemi_id, color, radius, false, transparent, false, segments);
    // Rotate hemisphere: (0, -radius, 0) -> (0, 0, radius) points to +Z.
    Quaternion y_to_z = rotateX(-M_PI / 2);  // Rotate -90 degrees around X axis.
    for (auto& shape : top_hemi->shapes()) {
        shape->rotate(y_to_z);
        shape->move(Vec3(0, 0, cylinder_height / 2.0));
    }
    capsule_obj->merge(top_hemi);

    // Bottom hemisphere: use top hemisphere (Y>=0) so pole points to -Z after rotation.
    std::string bottom_hemi_id = id + "_bottom_hemisphere";
    Object::Ptr bottom_hemi = generateHemisphere(bottom_hemi_id, color, radius, true, transparent, false, segments);
    // Rotate hemisphere: (0, radius, 0) -> (0, 0, -radius) points to -Z.
    for (auto& shape : bottom_hemi->shapes()) {
        shape->rotate(y_to_z);
        shape->move(Vec3(0, 0, -cylinder_height / 2.0));
    }
    capsule_obj->merge(bottom_hemi);

    return capsule_obj;
}
}  // namespace octo_flex
