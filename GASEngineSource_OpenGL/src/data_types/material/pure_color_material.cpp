#include "pure_color_material.h"
#include "data_types/shader_factory.h"
#include "ecs/component/camera_component.h"
#include "opengl/buffer_type.h"
#include "opengl/renderable_item.h"

void PureColorMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                       const std::shared_ptr<CameraComponent>& camera,
                                       const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kPureColorMaterial) {
        return;
    }
    std::shared_ptr<PureColorMaterial> material =
        std::dynamic_pointer_cast<PureColorMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["worldMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    uniform_values["cameraNearFar"] =
        CopyMemToByteArray(glm::vec2(camera->GetNear(), camera->GetFar()));

    uniform_values["pureColor"] = CopyMemToByteArray(this->default_color);
}