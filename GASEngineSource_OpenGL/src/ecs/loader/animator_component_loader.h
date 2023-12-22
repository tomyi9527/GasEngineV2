#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ecs/component/animator_component.h"
#include "ecs/loader/component_loader.h"
#include "rapidjson/document.h"
#include "utils/findfile.h"
#include "utils/json_maker.h"
#include "utils/resource_manager.h"

class AnimatorComponentLoader : public ComponentLoader {
 public:
    static std::unique_ptr<ComponentLoader> GenereateLoader(const rapidjson::Value& v) {
        return std::unique_ptr<AnimatorComponentLoader>(new AnimatorComponentLoader(v));
    }

 protected:
    AnimatorComponentLoader(const rapidjson::Value& v) : ComponentLoader(v) {
        json::GetMember(v, "uniqueID", unique_id);
        json::GetMember(v, "clips", clips);
    }

    pComponent Load(const std::vector<std::string>& path_hint,
                    const std::shared_ptr<Entity>& parent) override {
        pComponent component = ComponentFactory::Instance().Create(kAnimator);
        component->SetID(unique_id);
        if (parent) {
            parent->AddComponent(component);
        }
        for (const auto& clip : clips) {
            auto s = resource::ResourceManager::Instance().LoadStream(clip, path_hint);
            if (s && !s->fail()) {
                std::dynamic_pointer_cast<AnimatorComponent>(component)->LoadAnimator(*s);
            }
        }
        return component;
    }

 public:
    int64_t unique_id = -1;
    std::vector<std::string> clips;
};