#include "manipulator/manipulator.h"
#include <math.h>
#include <algorithm>
#include "ecs/component/camera_component.h"
#include "glm/gtx/euler_angles.hpp"

float Manipulator::GetHomeDistance(float scene_radius) {
    if (camera != nullptr && camera->camera_type == kPerspective) {
        float top = std::tan(camera->field_of_view * 0.5f) * camera->near;
        float bottom = -top;
        float left = bottom * camera->aspect;
        float right = top * camera->aspect;
        float vertical2 = std::abs(right - left) / camera->near / 2.0f;
        float horizontal2 = std::abs(top - bottom) / camera->near / 2.0f;
        float divisor = std::sin(std::atan2(std::min(horizontal2, vertical2), 1.0f));
        return scene_radius / divisor;
    } else {
        return 1.5 * scene_radius;
    }
}

float Manipulator::GetSpeedFactor() const { return std::max(distance, min_speed); }

void Manipulator::ComputeEyePosition() {
    this->eye = target + rotation * glm::vec3(0.0f, 0.0f, std::max(distance, min_distance));
}

void Manipulator::SetTarget(const glm::vec3& target) { this->target = target; }