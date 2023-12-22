#include "data_types/material/electric_material.h"
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

void ElectricMaterial::SetData(const ElectricMaterialData& data) {
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

    // // specular
    specular_enable = data.specular.enable;
    AssignIfLengthMatch(specular_factor, std::vector<double>{data.specular.factor});
    AssignIfLengthMatch(specular_default, data.specular.default_);
    AssignIfLengthMatch(specular_color, data.specular.tint);
    specular_map = TextureMap(data.specular.map);
    specular_map.LoadImageTexture(data.path_hint);

    // // roughness
    roughness_enable = data.roughness.enable;
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

bool ElectricMaterialData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "name", name) && ret;
    ret = json::GetMember(v, "culling", culling) && ret;
    ret = json::GetMember(v, "vertexShaderFile", vertexShaderFile) && ret;
    ret = json::GetMember(v, "fragmentShaderFile", fragmentShaderFile) && ret;
    ret = json::GetMember(v, "albedo", albedo) && ret;
    ret = json::GetMember(v, "specular", specular) && ret;
    ret = json::GetMember(v, "roughness", roughness) && ret;
    ret = json::GetMember(v, "displacement", displacement) && ret;
    ret = json::GetMember(v, "normal", normal) && ret;
    ret = json::GetMember(v, "ao", ao) && ret;
    ret = json::GetMember(v, "cavity", cavity) && ret;
    ret = json::GetMember(v, "transparency", transparency) && ret;
    ret = json::GetMember(v, "emissive", emissive) && ret;
    return ret;
}

void ElectricMaterialData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "culling", culling);
    js.AddMemberTo(v, "vertexShaderFile", vertexShaderFile);
    js.AddMemberTo(v, "fragmentShaderFile", fragmentShaderFile);
    js.AddMemberTo(v, "albedo", albedo);
    js.AddMemberTo(v, "specular", specular);
    js.AddMemberTo(v, "roughness", roughness);
    js.AddMemberTo(v, "displacement", displacement);
    js.AddMemberTo(v, "normal", normal);
    js.AddMemberTo(v, "ao", ao);
    js.AddMemberTo(v, "cavity", cavity);
    js.AddMemberTo(v, "transparency", transparency);
    js.AddMemberTo(v, "emissive", emissive);
}
