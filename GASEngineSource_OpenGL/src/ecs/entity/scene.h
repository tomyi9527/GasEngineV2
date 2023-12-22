#pragma once

#include <memory>
#include <set>
#include "ecs/entity_factory.h"

enum class kShadingMode : char {
    kWireframe,
    kPureColor,
    kBlinnPhong,
    kDielectric,
    kElectric,
    kMatcap
};

class CameraComponent;
class EnvironmentalLightComponent;
class DirectionalLightComponent;
class PunctualLightComponent;
class PointLightComponent;
class SpotLightComponent;
class AnimatorComponent;
class KeyframeAnimation;

enum kAnimationPlayMode { kAnimation_Play, kAnimation_Pause, kAnimation_Stop };

enum kAnimationLoopMode {
    kAnimation_Loop,
    kAnimation_Shuffle,
    kAnimation_Repeat,
    kAnimation_Once,
};

class Scene : public std::enable_shared_from_this<Scene> {
 public:
    static std::shared_ptr<Scene> GenerateObject(std::string in_name = "") {
        static int count = 0;
        if (in_name.empty()) {
            in_name = "scene_" + std::to_string(count);
        }
        return std::make_shared<Scene>(in_name);
    }

    std::shared_ptr<Scene> GetPtr() { return shared_from_this(); }
    void AppendEntityToRoot(const std::shared_ptr<Entity>& ptr) {
        if (root != nullptr) {
            root->AddChild(ptr);
            s_Traverse(root,
                       [this](const std::shared_ptr<Entity>& ptr) { ptr->SetScene(GetPtr()); },
                       nullptr);
        }
    }

    void FindObjectByPath(const std::string& path);
    void FindObjectByID(uint64_t id);
    void FindObjectByName(const std::string& name);
    void FindComponents(kComponentType type);
    void Cull();
    void UpdateGlobalSettings();
    void Update(float delta = 0.01f);

    void PickObject(int x, int y);
    void SetShadingMode(kShadingMode mode);

 public:
    Scene(const std::string& in_name) {
        root = EntityFactory::Instance().CreateEntity();
        root->name = in_name;
    }
    ~Scene() {
        // clear all entity
        if (root) {
            EntityFactory::Instance().Destroy(root);
        }
    }
    std::shared_ptr<Entity> root = nullptr;

 public:
    const std::shared_ptr<EnvironmentalLightComponent>& GetEnvironmentLight() const {
        return environmental_light;
    }
    const std::set<std::shared_ptr<DirectionalLightComponent>>& GetDirectionalLights() const {
        return directional_lights;
    }
    const std::set<std::shared_ptr<PunctualLightComponent>>& GetPunctualLights() const {
        return punctual_lights;
    }
    const std::set<std::shared_ptr<PointLightComponent>>& GetPointLights() const {
        return point_lights;
    }
    const std::set<std::shared_ptr<SpotLightComponent>>& GetSpotLights() const {
        return spot_lights;
    }
    void SetEnvironmentLight(
        const std::shared_ptr<EnvironmentalLightComponent>& in_environmental_light) {
        environmental_light = in_environmental_light;
    }
    void AddCamera(const std::shared_ptr<CameraComponent>& in_camera) {
        cameras.emplace(in_camera);
    }
    size_t GetCameraCount() const { return cameras.size(); }
    std::set<std::shared_ptr<CameraComponent>> GetCameras() const { return cameras; }
    void AddPunctualLight(const std::shared_ptr<PunctualLightComponent>& in_punctual_lights) {
        punctual_lights.emplace(in_punctual_lights);
    }
    void AddDirectionalLight(
        const std::shared_ptr<DirectionalLightComponent>& in_directional_light) {
        directional_lights.emplace(in_directional_light);
    }
    void AddSpotLight(const std::shared_ptr<SpotLightComponent>& in_spot_light) {
        spot_lights.emplace(in_spot_light);
    }

    void AddAnimator(const std::shared_ptr<AnimatorComponent>& animator) {
        if (animator) animators.emplace(animator);
    }

    kAnimationPlayMode GetAnimationPlayMode() const { return animator_play_mode; }
    kAnimationLoopMode GetAnimationLoopMode() const { return animator_loop_mode; }
    float GetAnimationPlaySpeed() const { return animator_play_speed; }
    void SetAnimationPlayMode(kAnimationPlayMode mode) { animator_play_mode = mode; }
    void SetAnimationLoopMode(kAnimationLoopMode mode) { animator_loop_mode = mode; }
    void SetAnimationPlaySpeed(float speed) { animator_play_speed = std::clamp(speed, 0.1f, 5.0f); }
    std::shared_ptr<AnimatorComponent> GetActiveAnimator() const;
    std::shared_ptr<KeyframeAnimation> GetActiveAnimation() const;
    float GetActiveAnimationProgress() const;
    void SetActiveAnimationProgress(float progress);

    void AddActiveAnimationClip(const std::shared_ptr<KeyframeAnimation>& clip) {
        if (clip) active_clips.emplace(clip);
    }
    void RemoveActiveAnimationClip(const std::shared_ptr<KeyframeAnimation>& clip) {
        auto it = active_clips.find(clip);
        if (it != active_clips.end()) {
            active_clips.erase(clip);
        }
    }
    void SetAnimationEnableClamp(bool in) { animation_enable_clamp = in; }
    float GetActiveAnimationTotalDuration() const;
    bool HasAnimator() const { return !animators.empty(); }

protected:
    int active_camera_index = -1;
    std::set<std::shared_ptr<CameraComponent>> cameras;

    std::shared_ptr<EnvironmentalLightComponent> environmental_light = nullptr;
    std::set<std::shared_ptr<DirectionalLightComponent>> directional_lights;
    std::set<std::shared_ptr<PunctualLightComponent>> punctual_lights;
    std::set<std::shared_ptr<PointLightComponent>> point_lights;
    std::set<std::shared_ptr<SpotLightComponent>> spot_lights;
    std::set<std::shared_ptr<AnimatorComponent>> animators;
    kAnimationPlayMode animator_play_mode = kAnimation_Play;
    kAnimationLoopMode animator_loop_mode = kAnimation_Loop;
    float animator_play_speed = 1.0f;
    bool animation_enable_clamp = false;
    std::set<std::shared_ptr<KeyframeAnimation>> active_clips;
};
using pScene = std::shared_ptr<Scene>;