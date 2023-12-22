#pragma once
#include <memory>
#include <vector>
#include "ecs/component_factory.h"

class Material;

class MeshRendererComponent : public Component {
 public:
    static std::shared_ptr<Component> GenerateComponent() {
        return std::make_shared<MeshRendererComponent>();
    }

    MeshRendererComponent() : Component(kMeshRenderer) {}

    void AddMaterial(const std::shared_ptr<Material>& mat) {
        if (mat != nullptr) materials.push_back(mat);
    }
    std::vector<std::shared_ptr<Material>> GetMaterials() { return materials; }

 private:
    std::vector<std::shared_ptr<Material>> materials;
};