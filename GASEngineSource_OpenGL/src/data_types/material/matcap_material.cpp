#include "matcap_material.h"
#include <memory>
#include <string>
#include <vector>
#include "data_types/material_loader.h"
#include "data_types/shader_factory.h"
#include "dielectric_material.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "glm/gtc/type_ptr.hpp"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/opengl_interface.h"
#include "opengl/renderable_item.h"
#include "shader_key_defination.h"

uint64_t MatCapMaterial::GenerateVertexShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateVertexShaderKey(item);

    if (displacement_enable == true) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENT;
    }
    if (displacement_map.Valid() && displacement_map.gl_texture_info) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DISPLACEMENTMAP;
    }

    return shader_key;
}

uint64_t MatCapMaterial::GenerateFragmentShaderKey(const RenderableItem& item) {
    uint64_t shader_key = SingleMaterial::GenerateFragmentShaderKey(item);

    if (matcap_enable) shader_key |= MatCap::kFSMacros::kFSMacros_MATCAP;
    if (matcap_map.Valid()) shader_key |= MatCap::kFSMacros::kFSMacros_MATCAP;
    // TODO(beanpliu): GasEngineV2中未使用下列信息。将来考虑检查下是否正确。
    // if (displacement_enable) shader_key |= MatCap::kFSMacros::kFSMacros_DISPLACEMENT;
    // if (displacement_map.Valid()) shader_key |= MatCap::kFSMacros::kFSMacros_DISPLACEMENTMAP;
    // if (normal_enable) shader_key |= MatCap::kFSMacros::kFSMacros_NORMAL;
    // if (normal_map.Valid()) shader_key |= MatCap::kFSMacros::kFSMacros_NORMALMAP;
    // if (transparency_enable) shader_key |= MatCap::kFSMacros::kFSMacros_TRANSPARENCY;
    // if (transparency_map.Valid()) shader_key |= MatCap::kFSMacros::kFSMacros_TRANSPARENCYMAP;

    return shader_key;
}

void MatCapMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                    const std::shared_ptr<CameraComponent>& camera,
                                    const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kMatCapMaterial) {
        return;
    }
    std::shared_ptr<MatCapMaterial> material =
        std::dynamic_pointer_cast<MatCapMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["worldMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    if (item.mesh->IsSkinned() && item.material->IsSkinned() &&
        AppGlobalResource::Instance().bone_texture &&
        !AppGlobalResource::Instance().bone_matrices_storage.data.empty()) {
        glit::GLTextureInfo::LoadBoneMatrixTexture(item.mesh);
        uniform_values["boneTexture"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->texture);
        uniform_values["boneTextureSize"] =
            CopyMemToByteArray(AppGlobalResource::Instance().bone_texture->width);
    }

    if (sp.fs_key & MatCap::kFSMacros::kFSMacros_MATCAP) {
        uniform_values["matCapColor"] = CopyMemToByteArray(material->matcap_color);
        uniform_values["matCapCurvature"] = CopyMemToByteArray(material->matcap_curvature);

        if (sp.fs_key & MatCap::kFSMacros::kFSMacros_MATCAPMAP && material->matcap_map.Valid()) {
            uniform_values["matCapMap"] =
                CopyMemToByteArray(material->matcap_map.gl_texture_info->texture);
        }
    }

    // TODO(beanpliu): GasEngineV2中未使用下列信息。将来考虑检查下是否正确。
    // if (sp.fs_key & MatCap::kFSMacros::kFSMacros_DISPLACEMENT) {
    //    uniform_values["displacementMapFactor"] =
    //    CopyMemToByteArray(material->displacement_factor); if (sp.fs_key &
    //    MatCap::kFSMacros::kFSMacros_DISPLACEMENTMAP &&
    //        material->displacement_map.Valid()) {
    //        uniform_values["displacementMap"] =
    //            CopyMemToByteArray(material->displacement_map.gl_texture_info->texture);
    //        uniform_values["displacementChannel"] =
    //            CopyMemToByteArray(material->displacement_map.pixel_channels);
    //    }
    //}

    // if (sp.fs_key & MatCap::kFSMacros::kFSMacros_NORMAL) {
    //    uniform_values["normalFactor"] = CopyMemToByteArray(material->normal_factor);
    //    uniform_values["normalFlipY"] = CopyMemToByteArray(material->normal_flip_y);

    //    if (sp.fs_key & MatCap::kFSMacros::kFSMacros_NORMALMAP &&
    //        material->normal_map.Valid()) {
    //        uniform_values["normalMap"] =
    //            CopyMemToByteArray(material->normal_map.gl_texture_info->texture);
    //    }
    //}

    // if (sp.fs_key & MatCap::kFSMacros::kFSMacros_TRANSPARENCY) {
    //    uniform_values["transparencyFactor"] = CopyMemToByteArray(material->transparency_factor);
    //    uniform_values["transparencyAlphaInvert"] =
    //        CopyMemToByteArray(material->transparency_alpha_invert);

    //    if (sp.fs_key & MatCap::kFSMacros::kFSMacros_TRANSPARENCYMAP &&
    //        material->transparency_map.Valid()) {
    //        uniform_values["transparencyMap"] =
    //            CopyMemToByteArray(material->transparency_map.gl_texture_info->texture);
    //        uniform_values["transparencyChannel"] =
    //            CopyMemToByteArray(material->transparency_map.pixel_channels);
    //    }
    //}
    uniform_values["outputLinear"] = CopyMemToByteArray(material->output_linear);
    uniform_values["rgbmRange"] = CopyMemToByteArray(material->rgbm_range);
}

void MatCapMaterial::SetData(const MatCapMaterialData& data) {
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
    matcap_enable = data.matsphere.enable;
    AssignIfLengthMatch(matcap_color, data.matsphere.tint);
    AssignIfLengthMatch(matcap_curvature, std::vector<double>{data.matsphere.curvature});
    matcap_map = TextureMap(data.matsphere.map);
    matcap_map.LoadImageTexture(data.path_hint);

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

    // // transparency
    transparency_enable = data.transparency.enable;
    transparency_alpha_invert[0] = data.transparency.invert ? 1 : 0;
    transparency_blend_mode[0] = data.transparency.mode;
    AssignIfLengthMatch(transparency_default, data.transparency.default_);
    AssignIfLengthMatch(transparency_factor, std::vector<double>{data.transparency.factor});
    transparency_map = TextureMap(data.transparency.map);
    transparency_map.LoadImageTexture(data.path_hint);
}

bool MatCapMaterialData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "name", name) && ret;
    ret = json::GetMember(v, "culling", culling) && ret;
    ret = json::GetMember(v, "vertexShaderFile", vertexShaderFile) && ret;
    ret = json::GetMember(v, "fragmentShaderFile", fragmentShaderFile) && ret;
    ret = json::GetMember(v, "reflectiveRatio", reflectiveRatio) && ret;
    ret = json::GetMember(v, "matsphere", matsphere) && ret;
    ret = json::GetMember(v, "displacement", displacement) && ret;
    ret = json::GetMember(v, "normal", normal) && ret;
    ret = json::GetMember(v, "transparency", transparency) && ret;
    return ret;
}

void MatCapMaterialData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "culling", culling);
    js.AddMemberTo(v, "vertexShaderFile", vertexShaderFile);
    js.AddMemberTo(v, "fragmentShaderFile", fragmentShaderFile);
    js.AddMemberTo(v, "reflectiveRatio", reflectiveRatio);
    js.AddMemberTo(v, "matsphere", matsphere);
    js.AddMemberTo(v, "displacement", displacement);
    js.AddMemberTo(v, "normal", normal);
    js.AddMemberTo(v, "transparency", transparency);
}
