#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "shader_key_defination.h"

class UVLayoutMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<UVLayoutMaterial>(new UVLayoutMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

 protected:
    UVLayoutMaterial() : SingleMaterial(kUVLayoutMaterial) {
        auto hint = GetShaderPathHint();
        vertex_shader_file =
            resource::ResourceManager::Instance().GenerateUri("uvLayoutVertex.glsl", hint);
        fragment_shader_file =
            resource::ResourceManager::Instance().GenerateUri("WireframeFragment.glsl", hint);
        state.flag_depth_test = true;
        state.culling = kCullingOff;

        is_transparency_enabled = false;
        is_wireframe = true;
        is_show_uvlayout = true;

        LoadAllContent();
    }

    glm::vec3 line_color = glm::vec3(0.0, 1.0, 1.0);
    glm::vec2 offset = glm::vec2(0.5, 0.5);
    glm::vec1 uv_scale = glm::vec1(1.0);
};