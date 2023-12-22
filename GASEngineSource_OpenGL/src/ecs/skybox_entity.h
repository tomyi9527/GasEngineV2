#pragma once
#include <memory>
#include <vector>
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component/mesh_renderer_component.h"
#include "ecs/entity_factory.h"

class SkyboxEntityFactory {
 public:
    static SkyboxEntityFactory& Instance() {
        static SkyboxEntityFactory instance;
        return instance;
    }

    std::shared_ptr<Entity> GenerateSkybox(const std::string& name = "skybox");

 protected:
    SkyboxEntityFactory() {}
};