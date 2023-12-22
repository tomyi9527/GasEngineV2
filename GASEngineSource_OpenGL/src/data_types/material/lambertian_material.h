#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "data_types/material_loader.h"

class LambertianMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<LambertianMaterial>(new LambertianMaterial);
    }

    uint64_t GenerateVertexShaderKey(const RenderableItem& item) override{
        return SingleMaterial::GenerateVertexShaderKey(item);
    }
    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override { return 0; }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

 protected:
    LambertianMaterial() : SingleMaterial(kLambertianMaterial) {
        auto hint = GetShaderPathHint();
        SetVertexShaderFile("PBRVertex.glsl");
        SetFragmentShaderFile("LambertFragment.glsl");
        // LoadAllContent();
    }

    glm::vec3 light_vec = glm::vec3(80.0, 100.0, 90.0);
};
