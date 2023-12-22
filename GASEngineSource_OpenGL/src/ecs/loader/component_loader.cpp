#include "component_loader.h"
#include "ecs/component_factory.h"
#include "ecs/loader/component_loader.h"
#include "ecs/loader/animator_component_loader.h"
#include "ecs/loader/mesh_filter_component_loader.h"
#include "ecs/loader/mesh_renderer_component_loader.h"

typedef std::unique_ptr<ComponentLoader> (*ComponentGenerator)(const rapidjson::Value&);
constexpr const ComponentGenerator generators[] = {
    AnimatorComponentLoader::GenereateLoader,
    MeshFilterComponentLoader::GenereateLoader,
    MeshRendererComponentLoader::GenereateLoader,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

static_assert((sizeof(generators) / sizeof(ComponentGenerator)) == kComponentTypeCount,
              "kComponentType should have same length as generators");

std::unique_ptr<ComponentLoader> ComponentLoaderFactory::CreateLoader(
    const std::string& name, const rapidjson::Value& json) {
    int type = Component::GetTypeByStr(name);
    if (type == -1) {
        return nullptr;
    }
    return CreateLoader((kComponentType)type, json);
}

std::unique_ptr<ComponentLoader> ComponentLoaderFactory::CreateLoader(
    kComponentType type, const rapidjson::Value& json) {
    std::unique_ptr<ComponentLoader> ret = nullptr;
    if (generators[type] == nullptr) {
        ret = nullptr;
    } else {
        ret = generators[type](json);
    }
    return ret;
}