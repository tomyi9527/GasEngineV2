#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"

class DepthMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<DepthMaterial>(new DepthMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

 protected:
    DepthMaterial() : SingleMaterial(kDepthMaterial) {
        auto hint = GetShaderPathHint();
        vertex_shader_file =
            resource::ResourceManager::Instance().GenerateUri("PBRVertex.glsl", hint);
        fragment_shader_file =
            resource::ResourceManager::Instance().GenerateUri("DepthFragment.glsl", hint);
        state.flag_depth_test = true;
        is_transparency_enabled = false;

        LoadAllContent();
    }
};