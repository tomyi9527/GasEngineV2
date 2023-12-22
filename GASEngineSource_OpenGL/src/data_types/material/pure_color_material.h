#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"

class PureColorMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<PureColorMaterial>(new PureColorMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

    void SetDefaultColor(const glm::vec4& color) { default_color = color; }

 protected:
    PureColorMaterial() : SingleMaterial(kPureColorMaterial), default_color(0.7, 0.7, 0.7, 1.0) {
        auto hint = GetShaderPathHint();
        vertex_shader_file =
            resource::ResourceManager::Instance().GenerateUri("PBRVertex.glsl", hint);
        fragment_shader_file =
            resource::ResourceManager::Instance().GenerateUri("PureColorFragment.glsl", hint);
        LoadAllContent();
    }
    glm::vec4 default_color;
};
