#pragma once
#include <assert.h>
#include <array>
#include <limits>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "utils/logger.h"

// using ValType = float;
// constexpr int dimension = 3;
class Box;

template <size_t dimension, typename ValType = float>
class BBOX {
 public:
    using VectorType = glm::vec<dimension, ValType, glm::defaultp>;
    VectorType min;
    VectorType max;

    BBOX() { Reset(); }
    ~BBOX() {}

    void Reset() {
        for (size_t i = 0; i < dimension; ++i) {
            min[i] = std::numeric_limits<ValType>::max();
            max[i] = std::numeric_limits<ValType>::lowest();
        }
    }
    void Reset(const VectorType& pos) {
        min = pos;
        max = pos;
    }
    VectorType GetCenter() const { return (min + max) / (ValType)(2); }
    VectorType GetRadius() const {
        VectorType ret = (max - min) / (ValType)(2);
        for (size_t i = 0; i < dimension; ++i) {
            if (ret[i] < 0) ret[i] = 0;
        }
        return ret;
    }
    ValType GetRadiusScalar() const { return glm::l2Norm(GetRadius()); }
    bool IsValid() const {
        for (size_t i = 0; i < dimension; ++i) {
            if (max[i] == std::numeric_limits<ValType>::lowest()) return false;
            if (min[i] == std::numeric_limits<ValType>::max()) return false;
            if (max[i] < min[i]) return false;
        }
        return true;
    }

    void FromVertices(const std::vector<ValType>& vert) {
        assert(vert.size() % dimension == 0);
        Reset();
        size_t vert_count = vert.size() / dimension;
        for (size_t i = 0; i < vert_count; ++i) {
            for (size_t j = 0; j < dimension; ++j) {
                min[j] = std::min(vert[i * dimension + j], min[j]);
                max[j] = std::max(vert[i * dimension + j], max[j]);
            }
        }
    }
    void FromBox(const Box& box);

    void Merge(const BBOX& rhs) {
        min = glm::min(min, rhs.min);
        max = glm::max(max, rhs.max);
    }
};

class Box {
 public:
    Box() { Reset(); }
    void Reset() {
        for (auto& m : vertices) {
            for (int i = 0; i < 3; ++i) {
                m[i] = 0.0;
            }
        }
    }
    void FromAABB(const BBOX<3>& bbox) {
        if (!bbox.IsValid()) {
            Reset();
            return;
        }
        // y high
        vertices[0] = glm::vec3(bbox.max.x, bbox.max.y, bbox.max.z);
        vertices[1] = glm::vec3(bbox.max.x, bbox.max.y, bbox.min.z);
        vertices[2] = glm::vec3(bbox.min.x, bbox.max.y, bbox.min.z);
        vertices[3] = glm::vec3(bbox.min.x, bbox.max.y, bbox.max.z);

        // u low
        vertices[4] = glm::vec3(bbox.min.x, bbox.min.y, bbox.min.z);
        vertices[5] = glm::vec3(bbox.min.x, bbox.min.y, bbox.max.z);
        vertices[6] = glm::vec3(bbox.max.x, bbox.min.y, bbox.max.z);
        vertices[7] = glm::vec3(bbox.max.x, bbox.min.y, bbox.min.z);
    }
    void ApplyTransform(const glm::mat4& mat) {
        for (auto& m : vertices) {
            m = mat * glm::vec4(m, 1.0);
        }
    }
    std::array<glm::vec3, 8> vertices;
    const std::array<glm::ivec2, 12> edges = {glm::ivec2{0, 1}, glm::ivec2{1, 2}, glm::ivec2{2, 3},
                                              glm::ivec2{3, 0}, glm::ivec2{4, 5}, glm::ivec2{5, 6},
                                              glm::ivec2{6, 7}, glm::ivec2{7, 4}, glm::ivec2{0, 6},
                                              glm::ivec2{1, 7}, glm::ivec2{2, 4}, glm::ivec2{3, 5}};
};

template <size_t dimension, typename ValType = float>
inline BBOX<dimension, ValType> Merge(const BBOX<dimension, ValType>& v1,
                                      const BBOX<dimension, ValType>& v2) {
    BBOX<dimension, ValType> ret = v1;
    ret.Merge(v2);
    return ret;
}

template <size_t dimension, typename ValType>
inline void BBOX<dimension, ValType>::FromBox(const Box& box) {
    if (dimension > 3) {
        LOG_ERROR("cannot set from box when dimension > 3");
        return;
    }
    Reset();
    size_t vert_count = box.vertices.size();
    for (size_t i = 0; i < vert_count; ++i) {
        for (size_t j = 0; j < dimension; ++j) {
            min[j] = std::min(box.vertices[i][j], min[j]);
            max[j] = std::max(box.vertices[i][j], max[j]);
        }
    }
}