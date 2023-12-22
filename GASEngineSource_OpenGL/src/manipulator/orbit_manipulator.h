#pragma once
#include <limits>
#include <memory>
#include "glm/glm.hpp"
#include "manipulator/manipulator.h"

#define _USE_MATH_DEFINES
#include <math.h>

class CameraComponent;
namespace glit {
namespace cpp {
class CustomGLFWContext;
}
}  // namespace glit

class OrbitManipulator : public Manipulator {
 public:
    constexpr static const float max_zoom_limit = std::numeric_limits<float>::max();
    constexpr static const float min_zoom_limit = 1.0e-4;
    constexpr static const float max_pitch_value = M_PI * 0.5f * 0.9f;
    constexpr static const float min_pitch_value = -M_PI * 0.5f * 0.9f;
    constexpr static const float max_yaw_value = M_PI;
    constexpr static const float min_yaw_value = -M_PI;

    void Update(float dt) override;
    // dx, dy is in [0, 1]
    void ComputeRotation(float dx, float dy);
    void ComputeZoom(float dz);
    void ComputePan(float dx, float dy);

 public:
    float current_yaw = 0.0f;
    float current_pitch = 0.0f;
};

class OrbitManipulatorStandardMouseKeyBoardController {
 public:
    OrbitManipulatorStandardMouseKeyBoardController(OrbitManipulator& target);
    ~OrbitManipulatorStandardMouseKeyBoardController() { UnBindGLFWContext(); }

    void BindGLFWContext(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context);
    void UnBindGLFWContext();

 public:
    void OnCursorMove(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void ProcessMouseMove(double dx, double dy);

 public:
    enum kControlMode {
        kControlModeRotate,
        kControlModePan,
        kControlModeZoom,
        kControlModeNone
    } mode = kControlModeNone;

    double xpos = -1.0;
    double ypos = -1.0;

    OrbitManipulator* ptr = nullptr;
    std::shared_ptr<glit::cpp::CustomGLFWContext> context;
};