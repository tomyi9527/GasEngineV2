#include "data_types/material/depth_material.h"
#include "data_types/shader_factory.h"
#include "ecs/component/camera_component.h"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/renderable_item.h"

void DepthMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                   const std::shared_ptr<CameraComponent>& camera,
                                   const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kDepthMaterial) {
        return;
    }
    std::shared_ptr<DepthMaterial> material =
        std::dynamic_pointer_cast<DepthMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["worldMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    uniform_values["cameraNearFar"] =
        CopyMemToByteArray(glm::vec2(camera->GetNear(), camera->GetFar()));

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
