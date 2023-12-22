#include "lambertian_material.h"
#include "opengl/renderable_item.h"
#include "ecs/component/camera_component.h"

void LambertianMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                        const std::shared_ptr<CameraComponent>& camera,
                                        const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kLambertianMaterial) {
        return;
    }
    std::shared_ptr<LambertianMaterial> material =
        std::dynamic_pointer_cast<LambertianMaterial>(item.material);

    glm::mat4 matrix_world_view = camera->GetViewMatrix() * item.world_matrix;
    glm::mat3 matrix_normal = matrix_world_view;
    uniform_values["modelMatrix"] = CopyMemToByteArray(item.world_matrix);
    uniform_values["viewMatrix"] = CopyMemToByteArray(camera->GetViewMatrix());
    uniform_values["worldViewMatrix"] = CopyMemToByteArray(matrix_world_view);
    uniform_values["projectionMatrix"] = CopyMemToByteArray(camera->GetProjectionMatrix());
    uniform_values["normalMatrix"] = CopyMemToByteArray(matrix_normal);
    
    uniform_values["lightVec"] = CopyMemToByteArray(light_vec);
}
