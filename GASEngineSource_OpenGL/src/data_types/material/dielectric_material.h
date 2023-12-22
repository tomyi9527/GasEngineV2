#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "data_types/material_loader.h"
#include "utils/json_maker.h"

class DielectricMaterialData : public json::JsonSerializeInterface {
 public:
    std::vector<std::string> path_hint;
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int culling = 0;
    std::string name;
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    MaterialPropertyData albedo, metalness, specularF0, roughness, displacement, normal, ao, cavity,
        transparency, emissive;
};

class DielectricMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<DielectricMaterial>(new DielectricMaterial(true));
    }

    uint64_t GenerateVertexShaderKey(const RenderableItem& item) override;
    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override;
    bool IsTransparencyEnabled() const override { return transparency_enable; }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

    void SetData(const DielectricMaterialData& data);

 protected:
    DielectricMaterial(bool is_dielectric) : SingleMaterial(kDielectricMaterial) {
        if (is_dielectric) {
            type = kDielectricMaterial;
            specular_f0_enable = true;
            metalness_enable = true;
            specular_enable = false;
        } else {
            type = kElectricMaterial;
            specular_f0_enable = false;
            metalness_enable = false;
            specular_enable = true;
        }
        auto hint = GetShaderPathHint();
        SetVertexShaderFile("PBRVertex.glsl");
        SetFragmentShaderFile("PBRFragment.glsl");
        // LoadAllContent();
    }

    float reflective_ratio = 0.5;
    bool is_dielectric = true;

    // Albedo
    bool albedo_enable = true;
    glm::vec3 albedo_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap albedo_map;
    glm::vec3 albedo_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 albedo_factor = glm::vec1(1.0f);

    // for dielectric
    // Metalness
    bool metalness_enable = false;
    glm::vec3 metalness_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap metalness_map;
    glm::vec1 metalness_factor = glm::vec1(0.0);

    // for dielectric
    // SpecularF0
    bool specular_f0_enable = false;
    glm::vec3 specular_f0_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap specular_f0_map;
    glm::vec1 specular_f0_factor = glm::vec1(0.0);

    // for electric
    // Specular
    bool specular_enable = false;
    glm::vec3 specular_default = glm::vec3(1.0, 1.0, 1.0);
    TextureMap specular_map;
    glm::vec3 specular_color = glm::vec3(1.0, 1.0, 1.0);
    glm::vec1 specular_factor = glm::vec1(0.0);

    // Roughness
    bool roughness_enable = false;
    glm::vec1 roughness_default = glm::vec1(1.0f);
    TextureMap roughness_map;
    glm::vec1 roughness_factor = glm::vec1(0.1f);
    glm::ivec1 roughness_invert = glm::ivec1(0);

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

    // AmbientOcclusion
    bool ambient_occlusion_enable = false;
    glm::vec4 ambient_occlusion_default = glm::vec4(1.0, 1.0, 1.0, 1.0);
    TextureMap ambient_occlusion_map;
    glm::vec1 ambient_occlusion_factor = glm::vec1(1.0f);
    glm::ivec1 ambient_occlusion_occlude_specular = glm::ivec1(1);

    // Cavity
    bool cavity_enable = false;
    glm::vec1 cavity_default = glm::vec1(1.0);
    TextureMap cavity_map;
    glm::vec1 cavity_factor = glm::vec1(1.0f);

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
};
