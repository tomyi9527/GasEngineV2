#include "manipulator/orbit_manipulator.h"
#include <math.h>
#include <algorithm>
#include <functional>
#include "ecs/component/camera_component.h"
#include "glm/gtx/euler_angles.hpp"
#include "opengl/opengl_interface.h"
#include "utils/quat_ext.h"

// this->distance = glm::distance(this->target, this->eye);

void OrbitManipulator::Update(float dt) {
    // update rotation
    // delayed_rotation.update(dt);
    // delayed_pan.update(dt);
    // delayed_zoom.update(dt);

    // update eye, target, y_up here
    ComputeEyePosition();

    if (camera) {
        camera->SetViewMatrix(eye, target, y_up);
    }
}

// modify rotation, and it will take effect on update
void OrbitManipulator::ComputeRotation(float dx, float dy) {
    float value;
    value = current_pitch - dy * 10.0;
    current_pitch = std::clamp(value, min_pitch_value, max_pitch_value);
    value = current_yaw - dx * 10.0;
    if (value > max_yaw_value) {
        value -= 2 * M_PI;
    } else if (value < min_yaw_value) {
        value += 2 * M_PI;
    }
    current_yaw = std::clamp(value, min_yaw_value, max_yaw_value);
    // std::cout << current_pitch << ", " << current_yaw << std::endl;
    rotation = glm::yawPitchRoll(current_yaw, current_pitch, 0.0f);
}

// modify zoom, and it will take effect on update
void OrbitManipulator::ComputeZoom(float dz) {
    float value = distance + GetSpeedFactor() * dz * 2.0;
    distance = std::clamp(value, min_zoom_limit, max_zoom_limit);
}

// modify pan, and it will take effect on update
void OrbitManipulator::ComputePan(float dx, float dy) {
    if (camera) {
        // 2 * tan(fov/2)
        float tan_fov_d_2 =
            camera->camera_type == kPerspective ? 2 * std::tan(camera->field_of_view / 2) : 1.0;
        float speed = tan_fov_d_2 * GetSpeedFactor();

        target += rotation[1] * speed * dy;
        target += rotation[0] * speed * -dx;
    }
}

OrbitManipulatorStandardMouseKeyBoardController::OrbitManipulatorStandardMouseKeyBoardController(
    OrbitManipulator& target) {
    ptr = &target;
}

void OrbitManipulatorStandardMouseKeyBoardController::OnCursorMove(double xpos, double ypos) {
    if (this->xpos < 0) {
        this->xpos = xpos;
    }
    if (this->ypos < 0) {
        this->ypos = ypos;
    }
    if (xpos != this->xpos || ypos != this->ypos) {
        ProcessMouseMove(xpos - this->xpos, ypos - this->ypos);
        this->xpos = xpos;
        this->ypos = ypos;
    }
}

void OrbitManipulatorStandardMouseKeyBoardController::OnMouseButton(int button, int action,
                                                                    int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (GLFW_PRESS == action)
            mode = kControlModeRotate;
        else if (GLFW_RELEASE == action)
            mode = kControlModeNone;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (GLFW_PRESS == action)
            mode = kControlModeZoom;
        else if (GLFW_RELEASE == action)
            mode = kControlModeNone;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (GLFW_PRESS == action)
            mode = kControlModePan;
        else if (GLFW_RELEASE == action)
            mode = kControlModeNone;
    }
}

void OrbitManipulatorStandardMouseKeyBoardController::ProcessMouseMove(double dx, double dy) {
    switch (mode) {
        case kControlModeRotate:
            ptr->ComputeRotation(dx, dy);
            break;
        case kControlModePan:
            ptr->ComputePan(dx, dy);
            break;
        case kControlModeZoom:
            ptr->ComputeZoom(dy);
            break;
        default:
            break;
    }
}
void OrbitManipulatorStandardMouseKeyBoardController::BindGLFWContext(
    const std::shared_ptr<glit::cpp::CustomGLFWContext>& context) {
    if (context == nullptr) {
        return;
    }
    this->context = context;
    context->on_mouse_button = [this](std::shared_ptr<glit::cpp::CustomGLFWContext> window,
                                      int button, int action,
                                      int mods) { this->OnMouseButton(button, action, mods); };
    context->on_cursor_move = [this](std::shared_ptr<glit::cpp::CustomGLFWContext> window, double x,
                                     double y) {
        this->OnCursorMove(x / window->width, y / window->height);
    };
}
void OrbitManipulatorStandardMouseKeyBoardController::UnBindGLFWContext() {
    if (context == nullptr) {
        return;
    }
    context->on_mouse_button = nullptr;
    context->on_cursor_move = nullptr;
}