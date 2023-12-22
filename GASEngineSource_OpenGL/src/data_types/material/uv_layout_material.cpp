#include "uv_layout_material.h"
#include "data_types/shader_factory.h"
#include "ecs/component/camera_component.h"
#include "opengl/renderable_item.h"

void UVLayoutMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                      const std::shared_ptr<CameraComponent>& camera,
                                      const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kUVLayoutMaterial) {
        return;
    }
    std::shared_ptr<UVLayoutMaterial> material =
        std::dynamic_pointer_cast<UVLayoutMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["modelMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["modelViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);

    uniform_values["uvScale"] = CopyMemToByteArray(uv_scale);
    uniform_values["lineColor"] = CopyMemToByteArray(line_color);
    uniform_values["offset"] = CopyMemToByteArray(offset);
}