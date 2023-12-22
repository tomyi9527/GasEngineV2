#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ecs/component_factory.h"
#include "rapidjson/document.h"
#include "utils/json_maker.h"

class Entity;

class ComponentLoader {
 public:
    ComponentLoader(const rapidjson::Value& v) {
        json::GetMember(v, "syncLoading", sync_loading);
        json::GetMember(v, "uniqueID", unique_id);
    }
    virtual std::shared_ptr<Component> Load(const std::vector<std::string>& path_hint,
                                            const std::shared_ptr<Entity>& parent = nullptr) = 0;
    virtual ~ComponentLoader() {}

 public:
    bool sync_loading = true;
    int unique_id = -1;
};

class ComponentLoaderFactory {
 public:
    static ComponentLoaderFactory& Instance() {
        static ComponentLoaderFactory instance;
        return instance;
    }

    std::unique_ptr<ComponentLoader> CreateLoader(const std::string& name,
                                                  const rapidjson::Value& json);
    std::unique_ptr<ComponentLoader> CreateLoader(kComponentType type,
                                                  const rapidjson::Value& json);

 protected:
    ComponentLoaderFactory() {}
};