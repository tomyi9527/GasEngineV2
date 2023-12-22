#pragma once

#include <memory>
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "utils/json_maker.h"

class Entity;
using pEntity = std::shared_ptr<Entity>;

class Scene;
using pScene = std::shared_ptr<Scene>;

class Component;
using pComponent = std::shared_ptr<Component>;

class GAS2Loader {
 public:
    static GAS2Loader GenerateLoader(const std::string& hint_path) { return GAS2Loader(hint_path); }

    pScene Load(const std::string& model_file);
    pScene Load(std::istream& s);

 protected:
    GAS2Loader(const std::string& hint_path) {
        if (!hint_path.empty()) hint_pathes.push_back(hint_path);
    }

    inline pEntity WalkSceneJson(const rapidjson::Value& v) {
        return WalkHierarchyRecursive(v, nullptr);
    }

    pEntity WalkHierarchyRecursive(const rapidjson::Value& v, const pEntity& parent);

    std::vector<std::string> hint_pathes;
};
