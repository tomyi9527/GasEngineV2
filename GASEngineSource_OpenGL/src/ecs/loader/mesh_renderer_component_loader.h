#pragma once
#include <memory>
#include <string>
#include <vector>
#include "data_types/material_loader.h"
#include "ecs/component/mesh_renderer_component.h"
#include "ecs/loader/component_loader.h"
#include "rapidjson/document.h"
#include "utils/json_maker.h"

class MeshRendererComponentLoader : public ComponentLoader {
 public:
    static std::unique_ptr<ComponentLoader> GenereateLoader(const rapidjson::Value& v) {
        return std::unique_ptr<MeshRendererComponentLoader>(new MeshRendererComponentLoader(v));
    }

 protected:
    MeshRendererComponentLoader(const rapidjson::Value& v) : ComponentLoader(v) {
        json::GetMember(v, "uniqueID", unique_id);
        json::GetMember(v, "materials", materials);
    }

    pComponent Load(const std::vector<std::string>& path_hint,
                    const std::shared_ptr<Entity>& parent) override {
        pComponent component = ComponentFactory::Instance().Create(kMeshRenderer);
        component->SetID(unique_id);
        if (parent) {
            parent->AddComponent(component);
        }
        for (const auto& m : materials) {
            auto stream = resource::ResourceManager::Instance().LoadStream(m, path_hint);
            if (stream && *stream) {
                auto pMaterial = MaterialLoader::CreateLoader(path_hint).LoadByIStream(*stream);
                std::dynamic_pointer_cast<MeshRendererComponent>(component)->AddMaterial(pMaterial);
            }
        }
        return component;
    }

 public:
    int64_t unique_id = -1;
    std::vector<std::string> materials;
};