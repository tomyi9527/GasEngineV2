#pragma once
#include <memory>
#include <vector>
#include "data_types/material_factory.h"
#include "ecs/component_factory.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl/renderable_item.h"

class Entity;
class OBJECT;
class SUBMESH;

enum kCameraType { kPerspective, kOrthographic };

class SkyboxComponent;

class CameraComponent : public Component {
 public:
    static std::shared_ptr<Component> GenerateComponent() {
        return std::make_shared<CameraComponent>();
    }

    CameraComponent()
        : Component(kCamera),
          camera_type(kPerspective),
          hdr(false),
          back_ground_color(0.8, 0.8, 0.8, 1.0),
          field_of_view(60.0),
          width(8.0),
          aspect(1.0),
          near(0.3),
          far(1000.0),
          rendering_order(0) {
        SetViewMatrix(glm::mat4(1.0));
        UpdateWorldMatrix();
        UpdateProjectionMatrix();
    }

    double GetNear() const { return near; }
    double GetFar() const { return far; }

    const glm::mat4& GetViewMatrix() const { return matrix_view; }
    const glm::mat4& GetWorldMatrix() const { return matrix_world; }
    void SetViewMatrix(const glm::mat4& view) { matrix_view = view; }
    void SetViewMatrix(const glm::vec3& camera_position, const glm::vec3& lookat,
                       const glm::vec3& up = glm::vec3(0, 1, 0)) {
        SetViewMatrix(glm::lookAt(camera_position, lookat, up));
    }
    void UpdateWorldMatrix() { matrix_world = glm::inverse(matrix_view); }

    const glm::mat4& GetProjectionMatrix() const { return matrix_projection; }
    void SetProjectionMatrix(const glm::mat4& projection) { matrix_projection = projection; }
    void SetProjectionMatrix(kCameraType in_type, double in_fov_or_width, double in_aspect,
                             double in_near, double in_far) {
        aspect = glm::abs(in_aspect);
        near = in_near;
        far = in_far;
        camera_type = in_type;
        if (camera_type == kPerspective) {
            field_of_view = in_fov_or_width;
        } else if (camera_type == kOrthographic) {
            width = in_fov_or_width;
        }
        UpdateProjectionMatrix();
    }
    void UpdateProjectionMatrix() {
        if (camera_type == kPerspective) {
            matrix_projection = glm::perspective(field_of_view, aspect, near, far);
        } else if (camera_type == kOrthographic) {
            double width_d2 = width / 2.0;
            double height_d2 = width * aspect / 2.0;
            matrix_projection = glm::ortho(-width_d2, width_d2, -height_d2, height_d2, near, far);
        }
    }

    const glm::mat4& GetViewProjectionMatrix() const { return matrix_view_projection; }
    void UpdateViewProjectionMatrix() { matrix_view_projection = matrix_projection * matrix_view; }

    std::shared_ptr<RenderableItem> GetSkyBox() const { return skybox; }

    std::shared_ptr<RenderableItem> GetHotspot() const { return hot_spot_item; }

    const std::vector<RenderableItem>& GetOpaqueList() const { return opaque_list; }

    const std::vector<RenderableItem>& GetTransparentList() const { return transparent_list; }

    const std::vector<RenderableItem>& GetHelperList() const { return helper_list; }

    void ClearRenderables() {
        opaque_list.clear();
        helper_list.clear();
        transparent_list.clear();
    }

    void SetHotSpotItem(const std::shared_ptr<OBJECT>& mesh,
                        const std::shared_ptr<Material>& material, const SUBMESH& submesh) {
        if (hot_spot_item == nullptr) {
            hot_spot_item = std::make_shared<RenderableItem>();
        }
        RenderableItem new_item;
        new_item.mesh = mesh;
        new_item.material = material;
        new_item.submesh = submesh;
        // new_item.matrix_world = matrix_world;
        std::swap(*hot_spot_item, new_item);
    }

    void AppendRenderables(
        const std::shared_ptr<OBJECT>& mesh, const std::shared_ptr<Material>& material,
        const SUBMESH& submesh,
        const std::shared_ptr<EnvironmentalLightComponent>& environmental_light,
        const std::set<std::shared_ptr<PunctualLightComponent>>& punctual_lights,
        const std::set<std::shared_ptr<DirectionalLightComponent>>& directional_lights,
        const std::set<std::shared_ptr<PointLightComponent>>& point_lights,
        const std::set<std::shared_ptr<SpotLightComponent>>& spot_lights,
        const glm::mat4& matrix_world, double depth, bool is_helper) {
        // if skybox
        if (material->GetType() == kSkyboxMaterial) {
            if (skybox == nullptr) {
                skybox = std::make_shared<RenderableItem>();
            }
            RenderableItem new_item;
            new_item.mesh = mesh;
            new_item.material = material;
            new_item.submesh = submesh;
            new_item.world_matrix = matrix_world;
            std::swap(*skybox, new_item);
            return;
        }

        // item
        RenderableItem item;
        item.mesh = mesh;
        item.material = material;
        item.submesh = submesh;
        item.environmental_light = environmental_light;
        item.punctual_lights = punctual_lights;
        item.directional_lights = directional_lights;
        item.point_lights = point_lights;
        item.spot_lights = spot_lights;
        item.depth = depth;
        item.world_matrix = matrix_world;

        // determine where to append this renderable item
        if (material != nullptr && material->IsTransparencyEnabled()) {
            transparent_list.push_back(std::move(item));
        } else if (is_helper) {
            helper_list.push_back(std::move(item));
        } else {
            opaque_list.push_back(std::move(item));
        }
    }

    // private:
    kCameraType camera_type;
    bool hdr;
    glm::vec4 back_ground_color;

    double field_of_view;  // for perspective
    double width;          // for orthographic

    double aspect;
    double near;
    double far;
    int rendering_order;

    glm::mat4 matrix_view;
    glm::mat4 matrix_projection;
    glm::mat4 matrix_view_projection;
    glm::mat4 matrix_world;

    // maybe not exists, so it's pointer
    std::shared_ptr<RenderableItem> skybox;
    std::shared_ptr<RenderableItem> hot_spot_item;

    // all renderables
    std::vector<RenderableItem> opaque_list;
    std::vector<RenderableItem> transparent_list;
    std::vector<RenderableItem> helper_list;
};