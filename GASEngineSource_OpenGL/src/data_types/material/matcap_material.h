#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "data_types/material_loader.h"
#include "utils/json_maker.h"

class MatCapMaterialData : public json::JsonSerializeInterface {
 public:
    std::vector<std::string> path_hint;
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int culling = 0;
    std::string name;
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    double reflectiveRatio = 0.8;
    MaterialPropertyData matsphere, displacement, normal, transparency;
};

class MatCapMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<MatCapMaterial>(new MatCapMaterial);
    }

    uint64_t GenerateVertexShaderKey(const RenderableItem& item) override;
    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override;
    bool IsTransparencyEnabled() const override { return transparency_enable; }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

    void SetData(const MatCapMaterialData& data);

 protected:
    MatCapMaterial() : SingleMaterial(kMatCapMaterial) {
        auto hint = GetShaderPathHint();
        // SetVertexShaderFile("MatCapVertex.glsl");
        SetVertexShaderFile("PBRVertex.glsl");
        SetFragmentShaderFile("MatCapFragment.glsl");
        // LoadAllContent();
    }

    float reflective_ratio = 0.5;

    // matcap
    bool matcap_enable = false;
    TextureMap matcap_map;
    glm::vec3 matcap_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 matcap_curvature = glm::vec1(0.0f);

    // Displacement
    bool displacement_enable = false;
    glm::vec1 displacement_default = glm::vec1(0.5f);
    TextureMap displacement_map;
    glm::vec1 displacement_factor = glm::vec1(0.5f);

    // Normal
    bool normal_enable = false;
    glm::vec3 normal_default = glm::vec3(0.5, 0.5, 1.0);
    TextureMap normal_map;
    glm::vec1 normal_factor = glm::vec1(1.0f);
    glm::vec1 normal_flip_y = glm::vec1(0.0f);

    // Transparency
    bool transparency_enable = false;
    glm::vec4 transparency_default = glm::vec4(1.0, 1.0, 1.0, 1.0);
    TextureMap transparency_map;
    glm::vec1 transparency_factor = glm::vec1(1.0f);
    glm::ivec1 transparency_alpha_invert = glm::ivec1(0);
    glm::ivec1 transparency_blend_mode = glm::ivec1(0);

    // other 原版实现内没有进行其他赋值
    glm::ivec1 output_linear = glm::ivec1(0);
    glm::vec1 rgbm_range = glm::vec1(0.0f);
};
