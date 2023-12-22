#include "blinn_phong_material.h"
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

uint64_t BlinnPhongMaterial::GenerateVertexShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateVertexShaderKey(item);

    if (displacement_enable == true) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENT;
    }
    if (displacement_map.Valid() && displacement_map.gl_texture_info) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENTMAP;
    }

    return shader_key;
}

uint64_t BlinnPhongMaterial::GenerateFragmentShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateFragmentShaderKey(item);

    if (albedo_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_ALBEDO;
    if (albedo_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_ALBEDOMAP;
    if (specular_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_SPECULAR;
    if (specular_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_SPECULARMAP;
    if (glossiness_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_GLOSSINES;
    if (glossiness_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_GLOSSINESMAP;
    if (displacement_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_DISPLACEMENT;
    if (displacement_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_DISPLACEMENTMAP;
    if (normal_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_NORMAL;
    if (normal_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_NORMALMAP;
    if (transparency_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_TRANSPARENCY;
    if (transparency_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_TRANSPARENCYMAP;
    if (emissive_enable) shader_key |= BlinnPhong::kFSMacros::kFSMacros_EMISSIVE;
    if (emissive_map.Valid()) shader_key |= BlinnPhong::kFSMacros::kFSMacros_EMISSIVEMAP;

    if (!item.directional_lights.empty()) {
        auto it = item.directional_lights.begin();
        if (*it != nullptr) {
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_DIRECTIONALLIGHTING;
        }
    }
    if (!item.point_lights.empty()) {
        auto it = item.point_lights.begin();
        if (*it != nullptr) {
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_POINTLIGHTING;
        }
    }
    if (!item.spot_lights.empty()) {
        auto it = item.spot_lights.begin();
        if (*it != nullptr) {
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_SPOTLIGHTING;
        }
    }

    if (item.environmental_light != nullptr &&
        item.environmental_light->HasDiffuseSphericalHarmonics()) {
        if (item.environmental_light->HasCubeMap() &&
            AppGlobalResource::Instance().SupportTextureLod()) {
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING;
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_CUBEMAP;
        } else if (item.environmental_light->HasPanorama()) {
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING;
            shader_key |= BlinnPhong::kFSMacros::kFSMacros_PANORAMA;
        }
    }

    return shader_key;
}

void BlinnPhongMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                        const std::shared_ptr<CameraComponent>& camera,
                                        const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kBlinnPhongMaterial) {
        return;
    }
    std::shared_ptr<BlinnPhongMaterial> material =
        std::dynamic_pointer_cast<BlinnPhongMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["worldMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_ALBEDO) {
        uniform_values["albedoColor"] = CopyMemToByteArray(material->albedo_color);
        uniform_values["albedoFactor"] = CopyMemToByteArray(material->albedo_factor);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_ALBEDOMAP &&
            material->albedo_map.Valid()) {
            uniform_values["albedoMap"] =
                CopyMemToByteArray(material->albedo_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_SPECULAR) {
        uniform_values["specularColor"] = CopyMemToByteArray(material->specular_color);
        uniform_values["specularFactor"] = CopyMemToByteArray(material->specular_factor);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_SPECULARMAP &&
            material->specular_map.Valid()) {
            uniform_values["specularMap"] =
                CopyMemToByteArray(material->specular_map.gl_texture_info->texture);
            uniform_values["specularChannel"] =
                CopyMemToByteArray(material->specular_map.pixel_channels);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_GLOSSINES) {
        uniform_values["glossinessFactor"] = CopyMemToByteArray(material->glossiness_factor);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_GLOSSINESMAP &&
            material->glossiness_map.Valid()) {
            uniform_values["glossinessMap"] =
                CopyMemToByteArray(material->glossiness_map.gl_texture_info->texture);
            uniform_values["glossinessChannel"] =
                CopyMemToByteArray(material->glossiness_map.pixel_channels);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_DISPLACEMENT) {
        uniform_values["displacementMapFactor"] = CopyMemToByteArray(material->displacement_factor);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_DISPLACEMENTMAP &&
            material->displacement_map.Valid()) {
            uniform_values["displacementMap"] =
                CopyMemToByteArray(material->displacement_map.gl_texture_info->texture);
            uniform_values["displacementChannel"] =
                CopyMemToByteArray(material->displacement_map.pixel_channels);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_NORMAL) {
        uniform_values["normalFactor"] = CopyMemToByteArray(material->normal_factor);
        uniform_values["normalFlipY"] = CopyMemToByteArray(material->normal_flip_y);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_NORMALMAP &&
            material->normal_map.Valid()) {
            uniform_values["normalMap"] =
                CopyMemToByteArray(material->normal_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_TRANSPARENCY) {
        state.blending = kBlendingType::kNormalBlend;
        uniform_values["transparencyFactor"] = CopyMemToByteArray(material->transparency_factor);
        uniform_values["transparencyAlphaInvert"] =
            CopyMemToByteArray(material->transparency_alpha_invert);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_TRANSPARENCYMAP &&
            material->transparency_map.Valid()) {
            uniform_values["transparencyMap"] =
                CopyMemToByteArray(material->transparency_map.gl_texture_info->texture);
            uniform_values["transparencyChannel"] =
                CopyMemToByteArray(material->transparency_map.pixel_channels);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_EMISSIVE) {
        uniform_values["emissiveColor"] = CopyMemToByteArray(material->emissive_color);
        uniform_values["emissiveFactor"] = CopyMemToByteArray(material->emissive_factor);

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_EMISSIVEMAP &&
            material->emissive_map.Valid()) {
            uniform_values["emissiveMap"] =
                CopyMemToByteArray(material->emissive_map.gl_texture_info->texture);
        }
    }

    if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_ENVIRONMENTALLIGHTING) {
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

        if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_CUBEMAP) {
            uniform_values["specularCubeMap"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map);
            uniform_values["specularCubeMapLODRange"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map_lod_range);
            uniform_values["specularCubeMapSize"] =
                CopyMemToByteArray(item.environmental_light->specular_cube_map_size);
        } else if (sp.fs_key & BlinnPhong::kFSMacros::kFSMacros_PANORAMA) {
            uniform_values["specularPanorama"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama);
            uniform_values["specularPanoramaLODRange"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama_lod_range);
            uniform_values["specularPanoramaSize"] =
                CopyMemToByteArray(item.environmental_light->specular_panorama_size);
        }
    }

    // 尚未支持
    ////Directional lighting
    // if (item.directional_lights.empty()) {
    //    if (item.directionalLight.position != = null &&
    //        item.directionalLight.color != = null &&
    //        item.directionalLight.intensity != = null &&
    //        item.directionalLight.direction != = null) {
    //        var directionalLight = {};
    //        //directionalLight.position = new Float32Array(item.directionalLight.position);
    //        directionalLight.color = new Float32Array(item.directionalLight.color);
    //        //directionalLight.intensity = new Float32Array([item.directionalLight.intensity]);
    //        directionalLight.ambientIntensity = new
    //        Float32Array([item.directionalLight.ambientIntensity]);
    //        directionalLight.diffuseIntensity = new
    //        Float32Array([item.directionalLight.diffuseIntensity]); directionalLight.specularPower
    //        = new Float32Array([item.directionalLight.specularPower]);
    //        directionalLight.specularIntensity = new
    //        Float32Array([item.directionalLight.specularIntensity]); directionalLight.direction =
    //        new Float32Array(item.directionalLight.direction); directionalLight.shininess = new
    //        Float32Array([item.directionalLight.shininess]); uniformValues['directionalLight'] =
    //        directionalLight;
    //        //uniform['length'] =
    //        //.isOn =
    //        //性能
    //        //max light count
    //    }
    //}

    // if (item.pointLight) {
    //    if (item.pointLight.position != = null &&
    //        item.pointLight.color != = null &&
    //        item.pointLight.intensity != = null &&
    //        item.pointLight.decay != = null) {
    //        var pointLight = {};
    //        pointLight.position = new Float32Array(item.pointLight.position);
    //        pointLight.color = new Float32Array(item.pointLight.color);
    //        // pointLight.intensity = new Float32Array([item.pointLight.intensity]);
    //        pointLight.ambientIntensity = new Float32Array([item.pointLight.ambientIntensity]);
    //        pointLight.diffuseIntensity = new Float32Array([item.pointLight.diffuseIntensity]);
    //        pointLight.specularPower = new Float32Array([item.pointLight.specularPower]);
    //        pointLight.specularIntensity = new Float32Array([item.pointLight.specularIntensity]);
    //        // pointLight.decay = new Float32Array([item.pointLight.decay]);
    //        pointLight.constant = new Float32Array([item.pointLight.constant]);
    //        pointLight.linear = new Float32Array([item.pointLight.linear]);
    //        pointLight.exp = new Float32Array([item.pointLight.exp]);
    //        pointLight.shininess = new Float32Array([item.pointLight.shininess]);
    //        uniformValues['pointLight'] = pointLight;
    //    }
    //}

    // if (item.spotLight) {
    //    if (item.spotLight.position != = null &&
    //        item.spotLight.color != = null &&
    //        item.spotLight.intensity != = null &&
    //        item.spotLight.direction != = null &&
    //        item.spotLight.angle != = null &&
    //        item.spotLight.distanceDecay != = null &&
    //        item.spotLight.angleDecay != = null) {
    //        var spotLight = {};
    //        spotLight.position = new Float32Array(item.spotLight.position);
    //        spotLight.color = new Float32Array(item.spotLight.color);
    //        spotLight.ambientIntensity = new Float32Array([item.spotLight.ambientIntensity]);
    //        spotLight.diffuseIntensity = new Float32Array([item.spotLight.diffuseIntensity]);
    //        spotLight.specularPower = new Float32Array([item.spotLight.specularPower]);
    //        spotLight.specularIntensity = new Float32Array([item.spotLight.specularIntensity]);
    //        // spotLight.intensity = new Float32Array([item.spotLight.intensity]);
    //        spotLight.direction = new Float32Array(item.spotLight.direction);
    //        spotLight.angle = new Float32Array([item.spotLight.angle]);
    //        spotLight.constant = new Float32Array([item.spotLight.constant]);
    //        spotLight.linear = new Float32Array([item.spotLight.linear]);
    //        spotLight.exp = new Float32Array([item.spotLight.exp]);
    //        spotLight.shininess = new Float32Array([item.spotLight.shininess]);
    //        uniformValues['spotLight'] = spotLight;
    //    }
    //}

    if (item.mesh->IsSkinned() && item.material->IsSkinned() &&
        AppGlobalResource::Instance().bone_texture &&
        !AppGlobalResource::Instance().bone_matrices_storage.data.empty()) {
        glit::GLTextureInfo::LoadBoneMatrixTexture(item.mesh);
        uniform_values["boneTexture"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->texture);
        uniform_values["boneTextureSize"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->width);
    }
}

void BlinnPhongMaterial::SetData(const BlinnPhongMaterialData& data) {
    // render state
    state.culling = static_cast<kCullingType>(data.culling);

    // base property
    name = data.name;
    SetVertexShaderFile(data.vertexShaderFile);
    SetFragmentShaderFile(data.fragmentShaderFile);
    LoadAllContent();

    // blinn phong property
    reflective_ratio = data.reflectiveRatio;
    // // albedo
    albedo_enable = data.albedo.enable;
    AssignIfLengthMatch(albedo_factor, std::vector<double>{data.albedo.factor});
    AssignIfLengthMatch(albedo_default, data.albedo.default_);
    AssignIfLengthMatch(albedo_color, data.albedo.tint);
    albedo_map = TextureMap(data.albedo.map);
    albedo_map.LoadImageTexture(data.path_hint);

    // // specular
    specular_enable = data.specular.enable;
    AssignIfLengthMatch(specular_factor, std::vector<double>{data.specular.factor});
    AssignIfLengthMatch(specular_default, data.specular.default_);
    AssignIfLengthMatch(specular_color, data.specular.tint);
    specular_map = TextureMap(data.specular.map);
    specular_map.LoadImageTexture(data.path_hint);

    // // glossiness
    glossiness_enable = data.glossiness.enable;
    AssignIfLengthMatch(glossiness_factor, std::vector<double>{data.glossiness.factor});
    AssignIfLengthMatch(glossiness_default, data.glossiness.default_);
    glossiness_map = TextureMap(data.glossiness.map);
    glossiness_map.LoadImageTexture(data.path_hint);

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

    // // light
    light_enable = data.light.enable;
    AssignIfLengthMatch(light_default, data.light.default_);
    AssignIfLengthMatch(light_color, data.light.tint);
    AssignIfLengthMatch(light_factor, std::vector<double>{data.light.factor});
    light_map = TextureMap(data.light.map);
    light_map.LoadImageTexture(data.path_hint);

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

bool BlinnPhongMaterialData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "name", name) && ret;
    ret = json::GetMember(v, "culling", culling) && ret;
    ret = json::GetMember(v, "vertexShaderFile", vertexShaderFile) && ret;
    ret = json::GetMember(v, "fragmentShaderFile", fragmentShaderFile) && ret;
    ret = json::GetMember(v, "reflectiveRatio", reflectiveRatio) && ret;
    ret = json::GetMember(v, "albedo", albedo) && ret;
    ret = json::GetMember(v, "specular", specular) && ret;
    ret = json::GetMember(v, "glossiness", glossiness) && ret;
    ret = json::GetMember(v, "displacement", displacement) && ret;
    ret = json::GetMember(v, "normal", normal) && ret;
    ret = json::GetMember(v, "light", light) && ret;
    ret = json::GetMember(v, "transparency", transparency) && ret;
    ret = json::GetMember(v, "emissive", emissive) && ret;
    return ret;
}

void BlinnPhongMaterialData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "culling", culling);
    js.AddMemberTo(v, "vertexShaderFile", vertexShaderFile);
    js.AddMemberTo(v, "fragmentShaderFile", fragmentShaderFile);
    js.AddMemberTo(v, "reflectiveRatio", reflectiveRatio);
    js.AddMemberTo(v, "albedo", albedo);
    js.AddMemberTo(v, "specular", specular);
    js.AddMemberTo(v, "glossiness", glossiness);
    js.AddMemberTo(v, "displacement", displacement);
    js.AddMemberTo(v, "normal", normal);
    js.AddMemberTo(v, "light", light);
    js.AddMemberTo(v, "transparency", transparency);
    js.AddMemberTo(v, "emissive", emissive);
}
