#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"
#include "shader_key_defination.h"

class SkyBoxMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<SkyBoxMaterial>(new SkyBoxMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override;

    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override;

    void SetSphericalHarmonics(const float exposure, const std::vector<glm::vec3>& sph) {
        light_exposure[0] = exposure;
        spherical_harmonics = sph;
    }
    void SetSolidColor(const float r, const float g, const float b) {
        SetSolidColor(glm::vec3(r, g, b));
    }
    void SetSolidColor(const glm::vec3& color) { solid_color = color; }
    void SetBackgroundExposure(const float exposure) { background_exposure[0] = exposure; }
    void SetImage(const TextureMap& in) { image = in; }
    void SetCubeMap(const TextureMap& map, const float size) {
        cube_map = map;
        cube_map_size[0] = size;
    }

    void ApplyPresetSolidColor();
    void ApplyPresetImage();
    void ApplyPresetCubeMap();
    void ApplyPresetAmbient();

 protected:
    SkyBoxMaterial() : SingleMaterial(kSkyboxMaterial) {
        auto hint = GetShaderPathHint();
        vertex_shader_file =
            resource::ResourceManager::Instance().GenerateUri("SkyboxVertex.glsl", hint);
        fragment_shader_file =
            resource::ResourceManager::Instance().GenerateUri("SkyboxFragment.glsl", hint);
        state.flag_depth_test = true;
        state.culling = kCullingOff;
        is_transparency_enabled = false;

        LoadAllContent();
    }

    SkyBox::kFSMacros background_type = SkyBox::kFSMacros_SOLIDCOLOR;

    // type image
    TextureMap image;
    // type solid color
    glm::vec3 solid_color = glm::vec3(0.0f, 0.0f, 0.0f);
    // type cube_map
    TextureMap cube_map;
    glm::vec1 cube_map_size = glm::vec1(1.0f);
    glm::vec1 light_exposure = glm::vec1(1.0f);
    glm::vec1 background_exposure = glm::vec1(1.0f);
    glm::vec1 orientation = glm::vec1(0.0f);
    // type ambient
    std::vector<glm::vec3> spherical_harmonics;
};