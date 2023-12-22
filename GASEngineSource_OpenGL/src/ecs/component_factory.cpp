#include "component_factory.h"
#include <cctype>
#include <map>
#include <memory>
#include "component/animator_component.h"
#include "component/camera_component.h"
#include "component/environmental_light_component.h"
#include "component/mesh_filter_component.h"
#include "component/mesh_renderer_component.h"
#include "utils/string_util.h"

// clang-format off
constexpr static const char* names[] = {
    "animator",
    "mesh_filter",
    "mesh_renderer",
    "camera",
    "punctual_light",
    "environmental_light",
    "resource",

    "directional_light",
    "point_light",
    "spot_light"
};

typedef std::shared_ptr<Component> (*GenerateComponent)();
constexpr static GenerateComponent generators[] = {
    AnimatorComponent::GenerateComponent,
    MeshFilterComponent::GenerateComponent,
    MeshRendererComponent::GenerateComponent,
    CameraComponent::GenerateComponent,
    nullptr,
    EnvironmentalLightComponent::GenerateComponent,
    nullptr,

    nullptr,
    nullptr,
    nullptr
};

// clang-format on

static_assert((sizeof(names) / sizeof(char*)) == kComponentTypeCount,
              "kComponentType should have same length as names");
static_assert((sizeof(generators) / sizeof(GenerateComponent)) == kComponentTypeCount,
              "kComponentType should have same length as generators");

std::string Component::GetTypeStr() const { return names[GetType()]; }

int Component::GetTypeByStr(std::string name) {
    static std::map<std::string, int> mapper;
    if (mapper.empty()) {
        for (int i = 0; i < kComponentTypeCount; ++i) {
            if (names[i] != nullptr) {
                std::string tmp = names[i];
                mapper.emplace(RemoveSpecialChar(ToLower(tmp), "-_ ."), i);
            }
        }
    }
    RemoveSpecialChar(ToLower(name), "-_ .");
    auto it = mapper.find(name);
    if (it != mapper.end()) {
        return it->second;
    } else {
        return -1;
    }
}

ComponentFactory::ComponentFactory() {}

std::shared_ptr<Component> ComponentFactory::Create(kComponentType type) {
    std::shared_ptr<Component> ret = nullptr;
    if (generators[type] == nullptr) {
        ret = nullptr;
    } else {
        ret = generators[type]();
    }
    // if (ret != nullptr) {
    //     used_components.emplace(ret);
    // }
    return ret;
}

// void ComponentFactory::Destroy(const std::shared_ptr<Component>& ptr) {
//     auto it = used_components.find(ptr);
//     if (it != used_components.end()) {
//         used_components.erase(it);
//     }
// }