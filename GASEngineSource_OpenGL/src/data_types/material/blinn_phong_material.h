#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "data_types/material_loader.h"
#include "utils/json_maker.h"

class BlinnPhongMaterialData : public json::JsonSerializeInterface {
 public:
    std::vector<std::string> path_hint;
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int culling = 0;
    std::string name;
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    double reflectiveRatio = 0.8;
    MaterialPropertyData albedo, specular, glossiness, displacement, normal, light, transparency,
        emissive;
};

class BlinnPhongMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<BlinnPhongMaterial>(new BlinnPhongMaterial);
    }

    uint64_t GenerateVertexShaderKey(const RenderableItem& item) override;
    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override;
    bool IsTransparencyEnabled() const override { return transparency_enable; }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

    void SetData(const BlinnPhongMaterialData& data);

 protected:
    BlinnPhongMaterial() : SingleMaterial(kBlinnPhongMaterial) {
        auto hint = GetShaderPathHint();
        SetVertexShaderFile("PBRVertex.glsl");
        SetFragmentShaderFile("BlinnPhongFragment.glsl");
        // LoadAllContent();
    }

    float reflective_ratio = 0.5;

    // Albedo
    bool albedo_enable = true;
    glm::vec3 albedo_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap albedo_map;
    glm::vec3 albedo_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 albedo_factor = glm::vec1(1.0f);

    // Specular
    bool specular_enable = false;
    glm::vec3 specular_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap specular_map;
    glm::vec3 specular_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 specular_factor = glm::vec1(0.0);

    // Glossiness
    bool glossiness_enable = false;
    glm::vec1 glossiness_default = glm::vec1(1.0f);
    TextureMap glossiness_map;
    glm::vec3 glossiness_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 glossiness_factor = glm::vec1(0.1f);

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

    // Emissive
    bool emissive_enable = false;
    glm::vec3 emissive_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap emissive_map;
    glm::vec3 emissive_color = glm::vec3(0.0, 0.0, 0.0);
    glm::vec1 emissive_factor = glm::vec1(0.0f);
    glm::vec1 emissive_multiplicative = glm::vec1(0.0f);

    // Light
    bool light_enable = false;
    glm::vec3 light_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap light_map;
    glm::vec3 light_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 light_factor = glm::vec1(1.0f);
};
