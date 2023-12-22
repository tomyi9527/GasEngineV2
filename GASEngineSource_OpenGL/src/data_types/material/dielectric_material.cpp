#include "dielectric_material.h"
#include <memory>
#include <string>
#include <vector>
#include "data_types/material_loader.h"
#include "data_types/shader_factory.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "glm/gtc/type_ptr.hpp"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/opengl_interface.h"
#include "opengl/renderable_item.h"
#include "shader_key_defination.h"

uint64_t DielectricMaterial::GenerateVertexShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateVertexShaderKey(item);

    if (displacement_enable == true) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENT;
    }
    if (displacement_map.Valid() && displacement_map.gl_texture_info) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENTMAP;
    }

    return shader_key;
}

uint64_t DielectricMaterial::GenerateFragmentShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateFragmentShaderKey(item);
    shader_key |= Dielectric::kFSMacros::kFSMacros_DIELECTRIC;

    // environmental lighting
    if (item.environmental_light != nullptr &&
        item.environmental_light->HasDiffuseSphericalHarmonics()) {
        if (item.environmental_light->HasCubeMap() &&
            AppGlobalResource::Instance().SupportTextureLod()) {
            shader_key |= Dielectric::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING;
            shader_key |= Dielectric::kFSMacros::kFSMacros_CUBEMAP;
        } else if (item.environmental_light->HasPanorama()) {
            shader_key |= Dielectric::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING;
            shader_key |= Dielectric::kFSMacros::kFSMacros_PANORAMA;
        }
    }

    if (albedo_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_ALBEDO;
    if (albedo_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_ALBEDOMAP;

    // dielectric
    if (metalness_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_METALNESS;
    if (metalness_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_METALNESSMAP;

    // dielectric
    if (specular_f0_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_SPECULARF0;
    if (specular_f0_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_SPECULARF0MAP;

    // electric
    if (specular_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_SPECULAR;
    if (specular_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_SPECULARMAP;

    if (roughness_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_ROUGHNESS;
    if (roughness_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_ROUGHNESSMAP;

    if (normal_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_NORMAL;
    if (normal_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_NORMALMAP;

    if (ambient_occlusion_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_AO;
    if (ambient_occlusion_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_AOMAP;

    if (cavity_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_CAVITY;
    if (cavity_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_CAVITYMAP;

    if (transparency_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_TRANSPARENCY;
    if (transparency_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_TRANSPARENCYMAP;

    if (emissive_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_EMISSIVE;
    if (emissive_map.Valid()) shader_key |= Dielectric::kFSMacros::kFSMacros_EMISSIVEMAP;

    //// viewmode
    // if (this.viewMode == 1) shaderKey |= (1 <<
    // GASEngine.DielectricMaterial.FSMacros.OUTPUTALBEDO); if (this.viewMode == 2) shaderKey |= (1
    // << GASEngine.DielectricMaterial.FSMacros.OUTPUTNORMALS);

    if (highlight_mask_enable) shader_key |= Dielectric::kFSMacros::kFSMacros_HIGHLIGHTMASK;

    return shader_key;
}

void DielectricMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                        const std::shared_ptr<CameraComponent>& camera,
                                        const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr) {
        return;
    }
    if (item.material->GetType() != kDielectricMaterial &&
        item.material->GetType() != kElectricMaterial) {
        return;
    }
    std::shared_ptr<DielectricMaterial> material =
        std::dynamic_pointer_cast<DielectricMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["worldMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    std::vector<float> camera_near_far = {static_cast<float>(camera->near),
                                          static_cast<float>(camera->far)};
    uniform_values["cameraNearFar"] = CopyMemToByteArray(camera_near_far);

    if (item.mesh->IsSkinned() && item.material->IsSkinned() &&
        AppGlobalResource::Instance().bone_texture &&
        !AppGlobalResource::Instance().bone_matrices_storage.data.empty()) {
        glit::GLTextureInfo::LoadBoneMatrixTexture(item.mesh);
        uniform_values["boneTexture"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->texture);
        uniform_values["boneTextureSize"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->width);
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING) {
        uniform_values["sph"] = CopyMemToByteArray(item.environmental_light->spherical_harmonics);
        uniform_values["specularIntegratedBRDF"] =
            CopyMemToByteArray(item.environmental_light->specular_integrated_brdf);
        uniform_values["environmentExposure"] =
            CopyMemToByteArray(item.environmental_light->environment_exposure);

        // rotationMatrix.makeRotationY(-threeJSScene.__skybox.backgroundConf.orientation * Math.PI
        // / 180.0);
        glm::mat4 rotation_matrix = glm::mat4(1.0f);
        glm::mat4 environment_matrix = camera->GetViewMatrix() * rotation_matrix;
        uniform_values["environmentMatrix"] = CopyMemToByteArray(environment_matrix);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_CUBEMAP) {
            uniform_values["specularCubeMap"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map);
            uniform_values["specularCubeMapLODRange"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map_lod_range);
            uniform_values["specularCubeMapSize"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map_size);
        } else if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_PANORAMA) {
            uniform_values["specularPanorama"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama);
            uniform_values["specularPanoramaLODRange"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama_lod_range);
            uniform_values["specularPanoramaSize"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama_size);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_ALBEDO) {
        uniform_values["albedoColor"] = CopyMemToByteArray(material->albedo_color);
        uniform_values["albedoFactor"] = CopyMemToByteArray(material->albedo_factor);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_ALBEDOMAP &&
            material->albedo_map.Valid()) {
            uniform_values["albedoMap"] =
                CopyMemToByteArray(material->albedo_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_METALNESS) {
        uniform_values["metalnessFactor"] = CopyMemToByteArray(material->metalness_factor);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_METALNESSMAP &&
            material->metalness_map.Valid()) {
            uniform_values["metalnessMap"] =
                CopyMemToByteArray(material->metalness_map.gl_texture_info->texture);
            uniform_values["metalnessChannel"] =
                CopyMemToByteArray(material->metalness_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_SPECULARF0) {
        uniform_values["specularF0Factor"] = CopyMemToByteArray(material->specular_f0_factor);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_SPECULARF0MAP &&
            material->metalness_map.Valid()) {
            uniform_values["specularF0Map"] =
                CopyMemToByteArray(material->specular_f0_map.gl_texture_info->texture);
            uniform_values["specularF0Channel"] =
                CopyMemToByteArray(material->specular_f0_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_SPECULAR) {
        uniform_values["specularFactor"] = CopyMemToByteArray(material->specular_factor);
        uniform_values["specularColor"] = CopyMemToByteArray(material->specular_color);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_SPECULARMAP &&
            material->specular_map.Valid()) {
            uniform_values["specularMap"] =
                CopyMemToByteArray(material->specular_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_ROUGHNESS) {
        uniform_values["roughnessFactor"] = CopyMemToByteArray(material->roughness_factor);
        uniform_values["roughnessInvert"] = CopyMemToByteArray(material->roughness_invert);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_ROUGHNESSMAP &&
            material->roughness_map.Valid()) {
            uniform_values["roughnessMap"] =
                CopyMemToByteArray(material->roughness_map.gl_texture_info->texture);
            uniform_values["roughnessChannel"] =
                CopyMemToByteArray(material->roughness_map.pixel_channels);
        }
    }

    // 注意此处为 vertex shader 宏
    if (sp.vs_key & Dielectric::kVSMacros::kVSMacros_DISPLACEMENT) {
        uniform_values["displacementMapFactor"] = CopyMemToByteArray(material->displacement_factor);

        if (sp.vs_key & Dielectric::kVSMacros::kVSMacros_DISPLACEMENT &&
            material->displacement_map.Valid()) {
            uniform_values["displacementMap"] =
                CopyMemToByteArray(material->displacement_map.gl_texture_info->texture);
            uniform_values["displacementChannel"] =
                CopyMemToByteArray(material->displacement_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_NORMAL) {
        uniform_values["normalFactor"] = CopyMemToByteArray(material->normal_factor);
        uniform_values["normalFlipY"] = CopyMemToByteArray(material->normal_flip_y);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_NORMALMAP &&
            material->normal_map.Valid()) {
            uniform_values["normalMap"] =
                CopyMemToByteArray(material->normal_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_AO) {
        uniform_values["aoFactor"] = CopyMemToByteArray(material->ambient_occlusion_factor);
        uniform_values["aoOccludeSpecular"] =
            CopyMemToByteArray(material->ambient_occlusion_occlude_specular);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_AOMAP &&
            material->ambient_occlusion_map.Valid()) {
            uniform_values["aoMap"] =
                CopyMemToByteArray(material->ambient_occlusion_map.gl_texture_info->texture);
            uniform_values["aoChannel"] =
                CopyMemToByteArray(material->ambient_occlusion_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_CAVITY) {
        uniform_values["cavityFactor"] = CopyMemToByteArray(material->roughness_factor);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_CAVITYMAP &&
            material->cavity_map.Valid()) {
            uniform_values["cavityMap"] =
                CopyMemToByteArray(material->cavity_map.gl_texture_info->texture);
            uniform_values["cavityChannel"] =
                CopyMemToByteArray(material->cavity_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_TRANSPARENCY) {
        state.blending = kBlendingType::kNormalBlend;
        uniform_values["transparencyFactor"] = CopyMemToByteArray(material->transparency_factor);
        uniform_values["transparencyAlphaInvert"] =
            CopyMemToByteArray(material->transparency_alpha_invert);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_TRANSPARENCYMAP &&
            material->transparency_map.Valid()) {
            uniform_values["transparencyMap"] =
                CopyMemToByteArray(material->transparency_map.gl_texture_info->texture);
            uniform_values["transparencyChannel"] =
                CopyMemToByteArray(material->transparency_map.pixel_channels);
        }
    }

    if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_EMISSIVE) {
        uniform_values["emissiveColor"] = CopyMemToByteArray(material->emissive_color);
        uniform_values["emissiveFactor"] = CopyMemToByteArray(material->emissive_factor);

        if (sp.fs_key & Dielectric::kFSMacros::kFSMacros_EMISSIVEMAP &&
            material->emissive_map.Valid()) {
            uniform_values["emissiveMap"] =
                CopyMemToByteArray(material->emissive_map.gl_texture_info->texture);
        }
    }

    if(sp.fs_key & (1 << Dielectric::kFSMacros::kFSMacros_HIGHLIGHTMASK))
    {
        uniform_values["highlightMaskColor"] = CopyMemToByteArray(material->highlight_mask_color);
    }
}

void DielectricMaterial::SetData(const DielectricMaterialData& data) {
    // render state
    state.culling = static_cast<kCullingType>(data.culling);

    // base property
    name = data.name;
    SetVertexShaderFile(data.vertexShaderFile);
    SetFragmentShaderFile(data.fragmentShaderFile);
    LoadAllContent();

    // // albedo
    albedo_enable = data.albedo.enable;
    AssignIfLengthMatch(albedo_factor, std::vector<double>{data.albedo.factor});
    AssignIfLengthMatch(albedo_default, data.albedo.default_);
    AssignIfLengthMatch(albedo_color, data.albedo.tint);
    albedo_map = TextureMap(data.albedo.map);
    albedo_map.LoadImageTexture(data.path_hint);

    // // metalness
    metalness_enable = data.metalness.enable;
    AssignIfLengthMatch(metalness_factor, std::vector<double>{data.metalness.factor});
    AssignIfLengthMatch(metalness_default, data.metalness.default_);
    metalness_map = TextureMap(data.metalness.map);
    metalness_map.LoadImageTexture(data.path_hint);

    // // specular_f0
    specular_f0_enable = data.specularF0.enable;
    AssignIfLengthMatch(specular_f0_factor, std::vector<double>{data.specularF0.factor});
    AssignIfLengthMatch(specular_f0_default, data.specularF0.default_);
    specular_f0_map = TextureMap(data.specularF0.map);
    specular_f0_map.LoadImageTexture(data.path_hint);

    // // roughness
    roughness_enable = data.roughness.enable;
    roughness_invert[0] = data.roughness.invert ? 1 : 0;
    AssignIfLengthMatch(roughness_factor, std::vector<double>{data.roughness.factor});
    AssignIfLengthMatch(roughness_default, data.roughness.default_);
    roughness_map = TextureMap(data.roughness.map);
    roughness_map.LoadImageTexture(data.path_hint);

    // // displacement
    displacement_enable = data.displacement.enable;
    AssignIfLengthMatch(displacement_factor, std::vector<double>{data.displacement.factor});
    AssignIfLengthMatch(displacement_default, data.displacement.default_);
    displacement_map = TextureMap(data.displacement.map);
    displacement_map.LoadImageTexture(data.path_hint);

    // // normal
    normal_enable = data.normal.enable;
    normal_flip_y[0] = data.normal.flipY ? 0.0f : 1.0f;
    AssignIfLengthMatch(normal_factor, std::vector<double>{data.normal.factor});
    AssignIfLengthMatch(normal_default, data.normal.default_);
    normal_map = TextureMap(data.normal.map);
    normal_map.LoadImageTexture(data.path_hint);

    // // ao
    ambient_occlusion_enable = data.ao.enable;
    ambient_occlusion_occlude_specular[0] = data.ao.occludeSpecular ? 0 : 1;
    AssignIfLengthMatch(ambient_occlusion_default, data.ao.default_);
    AssignIfLengthMatch(ambient_occlusion_factor, std::vector<double>{data.ao.factor});
    ambient_occlusion_map = TextureMap(data.ao.map);
    ambient_occlusion_map.LoadImageTexture(data.path_hint);

    // // cavity
    cavity_enable = data.cavity.enable;
    AssignIfLengthMatch(cavity_default, data.cavity.default_);
    AssignIfLengthMatch(cavity_factor, std::vector<double>{data.cavity.factor});
    cavity_map = TextureMap(data.cavity.map);
    cavity_map.LoadImageTexture(data.path_hint);

    // // transparency
    transparency_enable = data.transparency.enable;
    transparency_alpha_invert[0] = data.transparency.invert ? 1 : 0;
    transparency_blend_mode[0] = data.transparency.mode;
    AssignIfLengthMatch(transparency_default, data.transparency.default_);
    AssignIfLengthMatch(transparency_factor, std::vector<double>{data.transparency.factor});
    transparency_map = TextureMap(data.transparency.map);
    transparency_map.LoadImageTexture(data.path_hint);

    // // emissive
    emissive_enable = data.emissive.enable;
    emissive_multiplicative[0] = data.emissive.multiplicative ? 0.0f : 1.0f;
    AssignIfLengthMatch(emissive_color, data.emissive.tint);
    AssignIfLengthMatch(emissive_default, data.emissive.default_);
    AssignIfLengthMatch(emissive_factor, std::vector<double>{data.emissive.factor});
    emissive_map = TextureMap(data.emissive.map);
    emissive_map.LoadImageTexture(data.path_hint);
}

bool DielectricMaterialData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "name", name) && ret;
    ret = json::GetMember(v, "culling", culling) && ret;
    ret = json::GetMember(v, "vertexShaderFile", vertexShaderFile) && ret;
    ret = json::GetMember(v, "fragmentShaderFile", fragmentShaderFile) && ret;
    ret = json::GetMember(v, "albedo", albedo) && ret;
    ret = json::GetMember(v, "metalness", metalness) && ret;
    ret = json::GetMember(v, "specularF0", specularF0) && ret;
    ret = json::GetMember(v, "roughness", roughness) && ret;
    ret = json::GetMember(v, "displacement", displacement) && ret;
    ret = json::GetMember(v, "normal", normal) && ret;
    ret = json::GetMember(v, "ao", ao) && ret;
    ret = json::GetMember(v, "cavity", cavity) && ret;
    ret = json::GetMember(v, "transparency", transparency) && ret;
    ret = json::GetMember(v, "emissive", emissive) && ret;
    return ret;
}

void DielectricMaterialData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "culling", culling);
    js.AddMemberTo(v, "vertexShaderFile", vertexShaderFile);
    js.AddMemberTo(v, "fragmentShaderFile", fragmentShaderFile);
    js.AddMemberTo(v, "albedo", albedo);
    js.AddMemberTo(v, "metalness", metalness);
    js.AddMemberTo(v, "specularF0", specularF0);
    js.AddMemberTo(v, "roughness", roughness);
    js.AddMemberTo(v, "displacement", displacement);
    js.AddMemberTo(v, "normal", normal);
    js.AddMemberTo(v, "ao", ao);
    js.AddMemberTo(v, "cavity", cavity);
    js.AddMemberTo(v, "transparency", transparency);
    js.AddMemberTo(v, "emissive", emissive);
}
