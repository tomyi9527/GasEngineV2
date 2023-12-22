#pragma once

#include <map>
#include <memory>
#include <vector>
#include "data_types/material_loader.h"
#include "data_types/mesh_loader.h"
#include "data_types/shader_factory.h"
#include "glm/glm.hpp"
#include "opengl/opengl_interface.h"

class EnvironmentalLightComponent;
class DirectionalLightComponent;
class PunctualLightComponent;
class PointLightComponent;
class SpotLightComponent;
class CameraComponent;
class ShaderProgram;

class RenderableItem {
 public:
    std::shared_ptr<OBJECT> mesh;        // contains vertices and indices
    std::shared_ptr<Material> material;  // contains shader program, uniforms, etc
    SUBMESH submesh;

    double depth = 0.0;

    glm::mat4 world_matrix = glm::mat4(1.0f);
    std::shared_ptr<EnvironmentalLightComponent> environmental_light = nullptr;
    std::set<std::shared_ptr<DirectionalLightComponent>> directional_lights;
    std::set<std::shared_ptr<PunctualLightComponent>> punctual_lights;
    std::set<std::shared_ptr<PointLightComponent>> point_lights;
    std::set<std::shared_ptr<SpotLightComponent>> spot_lights;
};

namespace glit {

void SetSingleUniform(const ShaderVariableInfo& uniform_entry, const UniformValueStorage& value,
                      int& current_texture_slot);
GLVAOInfo CreateBuffer(const std::shared_ptr<OBJECT>& obj, const std::vector<kSECTION_TYPE>& types,
                       bool is_dynamic);
void UpdateBuffer(const std::shared_ptr<OBJECT>& obj, const std::vector<kSECTION_TYPE>& types,
                  bool is_dynamic);
bool SetUniformsRecursive(const std::map<std::string, ShaderVariableInfo>& sp_uniform,
                          const std::map<std::string, UniformValueStorage>& uniform_values,
                          int& current_texture_slot);
void RenderItem_V1(const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item);
void RenderPBR_V1(const std::set<std::shared_ptr<CameraComponent>>& cameras);
void RenderMesh(const RenderableItem& item, const ShaderProgram& sp,
                const std::map<std::string, UniformValueStorage>& uniform_values,
                std::vector<std::tuple<std::string, std::shared_ptr<GLVAOInfo>, kSECTION_TYPE>>
                    additional_attributes = {},
                bool is_wire_frame = false, bool show_uv_layout = false);

}  // namespace glit