#include "utils/quat_ext.h"
#include <time.h>
#include <iostream>
#include <tuple>
#include <vector>
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"
// for M_PI
#define _USE_MATH_DEFINES
#include <math.h>

float rand_float(float _min, float _max) {
    return _min + (_max - _min) * ((float)rand() / (float)(RAND_MAX));
}

int test_euler_matrix_conversion();

int main() {
    test_euler_matrix_conversion();
    // test_decompose();
    return 0;
}

void PrintMat4(const glm::mat4& mat) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) std::cout << mat[col][row] << " ";
        std::cout << std::endl;
    }
}
void PrintMat3(const glm::mat3& mat) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) std::cout << mat[col][row] << " ";
        std::cout << std::endl;
    }
}
void PrintVec3(const glm::vec3& vec) {
    for (int idx = 0; idx < 3; ++idx) {
        std::cout << vec[idx] << " ";
    }
    std::cout << std::endl;
}

// use degree input
// 注意，此处的mat3实际存储了 row-major 的顺序，而glm是 column-major，使用前需要转置。
std::vector<std::tuple<glm::vec3, kEulerMode, glm::mat3>> checker_euler_and_mat = {
    {{90.0f, 0.0f, 0.0f}, kEulerMode_ZYX, {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}}},
    {{0.0f, 90.0f, 0.0f}, kEulerMode_ZYX, {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}}},
    {{0.0f, 0.0f, 90.0f}, kEulerMode_ZYX, {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}},

    {{90.0f, 45.0f, 0.0f},
     kEulerMode_ZYX,
     {{0., -1., 0.}, {0.70710678, 0., 0.70710678}, {-0.70710678, 0., 0.70710678}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_ZYX,
     {{0.61237244, -0.2604026, 0.74645193},
      {0.35355339, 0.93472006, 0.03603338},
      {-0.70710678, 0.24184476, 0.66446302}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_ZXY,
     {{0.6928753, -0.35355339, 0.62842964},
      {0.67929002, 0.61237244, -0.40443179},
      {-0.24184476, 0.70710678, 0.66446302}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_XYZ,
     {{0.66446302, -0.24184476, 0.70710678},
      {0.62842964, 0.6928753, -0.35355339},
      {-0.40443179, 0.67929002, 0.61237244}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_XZY,
     {{0.66446302, -0.70710678, 0.24184476},
      {0.74645193, 0.61237244, -0.2604026},
      {0.03603338, 0.35355339, 0.93472006}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_YZX,
     {{0.61237244, -0.40443179, 0.67929002},
      {0.70710678, 0.66446302, -0.24184476},
      {-0.35355339, 0.62842964, 0.6928753}}},

    {{30.0f, 45.0f, 20.0f},
     kEulerMode_YXZ,
     {{0.93472006, 0.03603338, 0.35355339},
      {0.24184476, 0.66446302, -0.70710678},
      {-0.2604026, 0.74645193, 0.61237244}}},

};

int test_euler_matrix_conversion() {
    Rotation r;
    float epsilon = 0.00001f;
    for (const auto& m : checker_euler_and_mat) {
        r.FromEuler(std::get<1>(m), std::get<0>(m), true);
        auto result = r.AsMat3();
        auto reference = glm::transpose(std::get<2>(m));
        std::cout << "===========================" << std::endl;
        PrintMat3(result);
        std::cout << "-------------vs------------" << std::endl;
        PrintMat3(reference);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                float value_diff = std::abs(result[i][j] - reference[i][j]);
                assert(value_diff < epsilon);
            }
        }
        r.FromMatrix(reference);
        auto e = r.AsEuler(std::get<1>(m));
        auto e_result = glm::degrees(e.GetValue().first);
        auto e_reference = std::get<0>(m);
        std::cout << "===========================" << std::endl;
        PrintVec3(e_result);
        std::cout << "-------------vs------------" << std::endl;
        PrintVec3(e_reference);
        for (int i = 0; i < 3; ++i) {
            float value_diff = std::abs(e_result[i] - e_reference[i]);
            assert(value_diff < epsilon);
        }
    }
    std::cout << "===========================" << std::endl;
    std::cout << "euler between matrix result correct." << std::endl;

    return 0;
}
