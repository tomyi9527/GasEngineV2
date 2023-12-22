#pragma once
#include <limits>
#include <memory>
#include "ecs/component/camera_component.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "manipulator/manipulator.h"

class SimpleRotateManipulator : public Manipulator {
 public:
    void Update(float dt) override {
        time += dt;
        rotation = glm::toMat3(glm::angleAxis(time, glm::vec3(0, 1, 0)));
        ComputeEyePosition();
        if (camera) {
            camera->SetViewMatrix(eye, target, y_up);
        }
    }
    float time = 0.0f;
};