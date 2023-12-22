#pragma once
#include <memory>
#include <vector>
#include "data_types/material_factory.h"
#include "ecs/component_factory.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl/opengl_interface.h"

class Entity;

class EnvironmentalLightComponent : public Component {
 public:
    static std::shared_ptr<Component> GenerateComponent() {
        return std::make_shared<EnvironmentalLightComponent>();
    }

    EnvironmentalLightComponent() : Component(kEnvironmentalLight) {}

    void SetCubeMap(GLuint texture, int width, int height) {
        static const double ln2 = std::log(2);
        specular_cube_map = texture;
        specular_cube_map_size[0] = width;
        specular_cube_map_size[1] = height;

        specular_cube_map_lod_range[0] = std::log(width) / ln2;
        specular_cube_map_lod_range[1] = specular_cube_map_lod_range[0] - std::log(8) / ln2;
    }
    bool HasCubeMap() const { return specular_cube_map > 0; }

    bool HasDiffuseSphericalHarmonics() const { return spherical_harmonics.size() == 9; }

    void SetPanorama(GLuint texture, int width, int height) {
        static const double ln2 = std::log(2);
        specular_panorama = texture;
        specular_panorama_size[0] = width;
        specular_panorama_size[1] = height;

        specular_panorama_lod_range[0] = std::log(width) / ln2;
        specular_panorama_lod_range[1] = specular_panorama_lod_range[0] - std::log(32) / ln2;
    }
    bool HasPanorama() const { return specular_panorama > 0; }

    void SetDiffuseSphericalHarmonics(const float exposure, const std::vector<glm::vec3>& sph) {
        environment_exposure[0] = exposure;
        spherical_harmonics = sph;
    }

    void SetIntegratedBRDF(GLuint texture) { specular_integrated_brdf = texture; }

 public:
    glm::vec1 environment_exposure = glm::vec2(1.0f);

    GLuint specular_cube_map = 0;
    glm::vec2 specular_cube_map_size = glm::vec2(0.0, 0.0);
    glm::vec2 specular_cube_map_lod_range = glm::vec2(0.0, 0.0);

    GLuint specular_panorama = 0;
    glm::vec2 specular_panorama_size = glm::vec2(0.0, 0.0);
    glm::vec2 specular_panorama_lod_range = glm::vec2(0.0, 0.0);

    GLuint specular_integrated_brdf = 0;

    std::vector<glm::vec3> spherical_harmonics;
};