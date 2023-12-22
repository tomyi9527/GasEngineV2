#pragma once

#include "ecs/component/mesh_filter_component.h"
#include "ecs/component_factory.h"
#include "glm/glm.hpp"
#include "mesh_loader.h"

#define _USE_MATH_DEFINES
#include <math.h>

class SphereMesh {
 public:
    // radius 1.0f
    static pOBJECT UnitMesh() {
        static pOBJECT ret = Generate();
        return ret;
    }
    static pOBJECT Generate(float radius = 1.0f, int width_segments = 100,
                            int height_segments = 100, float phi_start = 0,
                            float phi_length = 2 * M_PI, float theta_start = 0,
                            float theta_length = M_PI) {
        width_segments = std::max(3, width_segments);
        height_segments = std::max(2, height_segments);
        float theta_end = theta_start + theta_length;

        pOBJECT obj = OBJECT::GenerateOBJECT();
        std::vector<std::vector<int>> grid;
        int index = 0;

        for (int iy = 0; iy <= height_segments; ++iy) {
            std::vector<int> grid_row;
            float v = static_cast<float>(iy) / height_segments;
            for (int ix = 0; ix <= width_segments; ++ix) {
                float u = static_cast<float>(ix) / width_segments;
                glm::vec3 vertex(-radius * std::cos(phi_start + u * phi_length) *
                                     std::sin(theta_start + v * theta_length),
                                 radius * std::cos(theta_start + v * theta_length),
                                 radius * std::sin(phi_start + u * phi_length) *
                                     std::sin(theta_start + v * theta_length));
                // vertex:
                obj->position.push_back(vertex.x);
                obj->position.push_back(vertex.y);
                obj->position.push_back(vertex.z);
                // normal
                vertex = glm::normalize(vertex);
                obj->normal0.push_back(vertex.x);
                obj->normal0.push_back(vertex.y);
                obj->normal0.push_back(vertex.z);
                // uv
                obj->uv0.push_back(u);
                obj->uv0.push_back(1 - v);
                // grid
                grid_row.push_back(index++);
            }
            grid.push_back(std::move(grid_row));
        }

        // indices
        for (int iy = 0; iy < height_segments; ++iy) {
            for (int ix = 0; ix < width_segments; ++ix) {
                glm::ivec4 rectangle(grid[iy][ix + 1], grid[iy][ix], grid[iy + 1][ix],
                                     grid[iy + 1][ix + 1]);
                // 点退化(重叠)
                if (iy != 0 || theta_start > 0) {
                    obj->indices.push_back(rectangle[0]);
                    obj->indices.push_back(rectangle[1]);
                    obj->indices.push_back(rectangle[3]);
                }
                if (iy != height_segments - 1 || theta_end < M_PI) {
                    obj->indices.push_back(rectangle[1]);
                    obj->indices.push_back(rectangle[2]);
                    obj->indices.push_back(rectangle[3]);
                }
            }
        }
        SUBMESH sm(0, obj->indices.size() / 3);
        obj->submesh.push_back(sm);

        // 表示Mesh内有这个信息。
        obj->section_item_map.emplace(kPOSITION, SECTION_ITEM());
        obj->section_item_map.emplace(kINDEX, SECTION_ITEM());
        obj->section_item_map.emplace(kNORMAL0, SECTION_ITEM());
        obj->section_item_map.emplace(kUV0, SECTION_ITEM());

        return obj;
    }
};

// 但目前实际是按八面体的做法做的
class PyramidMesh {
 public:
    // height 1.0f, diagonal_radius 1.0f
    static pOBJECT UnitMesh() { 
        static pOBJECT ret = Generate();
        return ret;
    }
    static pOBJECT Generate(const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 1.0f), float size = 1.0f) {
        glm::vec3 norm = glm::normalize(pos);
        glm::vec3 midpos(0, 0, 0);
        glm::vec3 rv(norm.x, norm.y - 1.0f, norm.z);
        glm::vec3 offset1 = glm::normalize(glm::cross(norm, rv));
        glm::vec3 offset2 = glm::cross(norm, offset1);
        offset1 *= size;
        offset2 *= size;

        pOBJECT obj = OBJECT::GenerateOBJECT();
        // 为了flat照明
        obj->position = {
            midpos.x - offset1.x,
            midpos.y - offset1.y,
            midpos.z - offset1.z,
            midpos.x - offset1.x,
            midpos.y - offset1.y,
            midpos.z - offset1.z,
            midpos.x - offset1.x,
            midpos.y - offset1.y,
            midpos.z - offset1.z,
            midpos.x - offset1.x,
            midpos.y - offset1.y,
            midpos.z - offset1.z,
            midpos.x + offset1.x,
            midpos.y + offset1.y,
            midpos.z + offset1.z,
            midpos.x + offset1.x,
            midpos.y + offset1.y,
            midpos.z + offset1.z,
            midpos.x + offset1.x,
            midpos.y + offset1.y,
            midpos.z + offset1.z,
            midpos.x + offset1.x,
            midpos.y + offset1.y,
            midpos.z + offset1.z,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            midpos.x - offset2.x,
            midpos.y - offset2.y,
            midpos.z - offset2.z,
            midpos.x - offset2.x,
            midpos.y - offset2.y,
            midpos.z - offset2.z,
            midpos.x - offset2.x,
            midpos.y - offset2.y,
            midpos.z - offset2.z,
            midpos.x - offset2.x,
            midpos.y - offset2.y,
            midpos.z - offset2.z,
            midpos.x + offset2.x,
            midpos.y + offset2.y,
            midpos.z + offset2.z,
            midpos.x + offset2.x,
            midpos.y + offset2.y,
            midpos.z + offset2.z,
            midpos.x + offset2.x,
            midpos.y + offset2.y,
            midpos.z + offset2.z,
            midpos.x + offset2.x,
            midpos.y + offset2.y,
            midpos.z + offset2.z,
            pos.x,
            pos.y,
            pos.z,
            pos.x,
            pos.y,
            pos.z,
            pos.x,
            pos.y,
            pos.z,
            pos.x,
            pos.y,
            pos.z,
        };
        obj->indices = {0, 12, 20, 21, 13, 4, 5, 16, 22, 23, 17, 1,
                        2, 8,  14, 15, 9,  6, 7, 10, 18, 19, 11, 3};
        obj->normal0 = obj->ComputeNormal();
        SUBMESH sm(0, obj->indices.size() / 3);
        obj->submesh.push_back(sm);

        // 表示Mesh内有这个信息。
        obj->section_item_map.emplace(kPOSITION, SECTION_ITEM());
        obj->section_item_map.emplace(kINDEX, SECTION_ITEM());
        obj->section_item_map.emplace(kNORMAL0, SECTION_ITEM());

        return obj;
    }
};