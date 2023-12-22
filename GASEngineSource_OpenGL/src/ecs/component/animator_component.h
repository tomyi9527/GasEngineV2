#pragma once
#include <map>
#include <memory>
#include <random>
#include <string>
#include "data_types/keyframe_animator_loader.h"
#include "data_types/mesh_loader.h"
#include "ecs/component_factory.h"
#include "ecs/entity_factory.h"

class AnimatorComponent : public Component {
 public:
    static std::shared_ptr<Component> GenerateComponent() {
        return std::make_shared<AnimatorComponent>();
    }

    AnimatorComponent() : Component(kAnimator), gen(rd()) {}

    const std::vector<std::shared_ptr<KeyframeAnimation>>& GetAnimatorClips() const {
        return animator_;
    }
    std::shared_ptr<KeyframeAnimation> GetAnimatorClip(const std::string& name) const {
        auto it = animator_name_indexed.find(name);
        if (it != animator_name_indexed.end())
            return it->second;
        else
            return nullptr;
    }
    const std::shared_ptr<KeyframeAnimation>& GetActiveAnimatorClip() const { return active_clip; }
    void AddAnimator(const std::shared_ptr<KeyframeAnimation>& in) {
        if (in) {
            auto result = animator_name_indexed.emplace(in->clip_name, in);
            if (!result.second) {
                LOG_ERROR("animator name %s already exists", in->clip_name.c_str());
            } else {
                animator_.emplace_back(in);
                if (parent_entity_ != nullptr) {
                    // gas2 loader 内加载完成后需再次调用
                    in->LinkToObjectsAndProperties(parent_entity_);
                }
            }
        }
    }
    void RemoveAnimator(const std::string& name) {
        auto it = animator_name_indexed.find(name);
        if (it != animator_name_indexed.end()) {
            auto vec_it = std::find(animator_.begin(), animator_.end(), it->second);
            if (vec_it != animator_.end()) {
                animator_.erase(vec_it);
            }
            animator_name_indexed.erase(it);
        }
    }
    void LoadAnimator(std::istream& s);

    void Play(const std::string& name) { Play(GetAnimatorClip(name)); }
    void Play(const std::shared_ptr<KeyframeAnimation>& clip) {
        if (clip) {
            LOG_INFO("playing clip : %s for %f", clip->clip_name.c_str(), clip->Duration());
            active_clip = clip;
            active_clip->SetProgress(0.0f);
        }
    }
    void PlayFirst() {
        if (animator_.empty()) {
            return;
        }
        Play(animator_.front());
    }
    void PlayNext(std::shared_ptr<KeyframeAnimation> clip) {
        if (!clip) {
            clip = active_clip;
        }
        if (!clip || animator_.empty()) {
            return;
        }
        auto it = std::find(animator_.begin(), animator_.end(), clip);
        if (it == animator_.end()) {
            Play(animator_.front());
        } else {
            it++;
            if (it == animator_.end()) {
                Play(animator_.front());
            } else {
                Play(*it);
            }
        }
    }
    void PlayRandom() {
        if (animator_.empty()) {
            return;
        }
        std::uniform_int_distribution<> distrib(0, animator_.size() - 1);
        int choice = distrib(gen);
        Play(animator_[choice]);
    }
    void Stop(const std::string& name = "") {
        if (active_clip && (name == active_clip->clip_name || name.empty())) {
            active_clip = nullptr;
        }
    }

 private:
    std::vector<std::shared_ptr<KeyframeAnimation>> animator_;
    std::map<std::string, std::shared_ptr<KeyframeAnimation>> animator_name_indexed;
    std::shared_ptr<KeyframeAnimation> active_clip;
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen;       // Standard mersenne_twister_engine seeded with rd()
};