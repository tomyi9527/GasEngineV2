#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ecs/component/mesh_filter_component.h"
#include "ecs/loader/component_loader.h"
#include "rapidjson/document.h"
#include "utils/json_maker.h"
#include "utils/resource_manager.h"

class MeshFilterComponentLoader : public ComponentLoader {
 public:
    static std::unique_ptr<ComponentLoader> GenereateLoader(const rapidjson::Value& v) {
        return std::unique_ptr<MeshFilterComponentLoader>(new MeshFilterComponentLoader(v));
    }

 protected:
    MeshFilterComponentLoader(const rapidjson::Value& v) : ComponentLoader(v) {
        json::GetMember(v, "uniqueID", unique_id);
        json::GetMember(v, "mesh", mesh);
        json::GetMember(v, "bbox", bbox);
    }

    pComponent Load(const std::vector<std::string>& path_hint,
                    const std::shared_ptr<Entity>& parent) override {
        pComponent component = ComponentFactory::Instance().Create(kMeshFilter);
        component->SetID(unique_id);
        if (parent) {
            parent->AddComponent(component);
        }
        auto s = resource::ResourceManager::Instance().LoadStream(mesh, path_hint);
        if (s && !s->fail()) {
            std::dynamic_pointer_cast<MeshFilterComponent>(component)->LoadMesh(*s);
        }
        return component;
    }

 public:
    int64_t unique_id = -1;
    std::string mesh;
    std::vector<std::vector<double>> bbox;
};