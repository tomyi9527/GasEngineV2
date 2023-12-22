#pragma once
#include <limits>
#include <memory>
#include "glm/glm.hpp"

class CameraComponent;

class Manipulator {
    constexpr static const float min_distance = 1.0e-8;

 public:
    void InitialSettings(const std::shared_ptr<CameraComponent>& camera, const glm::vec3& center,
                         float radius) {
        this->camera = camera;
        distance = GetHomeDistance(radius);
        SetTarget(center);
        ComputeEyePosition();
    }

    virtual ~Manipulator() {}
    virtual void Update(float dt) = 0;

    float distance = 0.0f;
    glm::mat3 rotation = glm::mat3(1.0f);
    float min_speed = 0.0f;

    glm::vec3 target = glm::vec3(0.0f);
    glm::vec3 eye = glm::vec3(0.0f);
    glm::vec3 y_up = glm::vec3(0.0f, 1.0f, 0.0f);
    std::shared_ptr<CameraComponent> camera;

 protected:
    float GetHomeDistance(float scene_radius);
    float GetSpeedFactor() const;
    void ComputeEyePosition();
    void SetTarget(const glm::vec3& target);
};
