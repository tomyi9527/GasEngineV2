#include "gas2_loader.h"
#include <filesystem>
#include <istream>
#include <iterator>
#include <string>
#include "component_loader.h"
#include "data_types/mesh_loader.h"
#include "ecs/enhancement/skeleton_manager.h"
#include "ecs/component/animator_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/entity/scene.h"
#include "utils/json_maker.h"
#include "utils/resource_manager.h"

pScene GAS2Loader::Load(const std::string& model_file) {
    auto file = resource::ResourceManager::Instance().LoadStream(model_file);
    std::filesystem::path file_path = model_file;
    if (file_path.has_parent_path()) {
        hint_pathes.push_back(file_path.parent_path().string());
    }
    if (!file || file->fail()) {
        return nullptr;
    } else {
        return Load(*file);  // stream将在此句后释放
    }
}

pScene GAS2Loader::Load(std::istream& s) {
    std::string input;
    input.assign(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>());

    rapidjson::Document doc;
    doc.Parse(input.data(), input.size());
    if (!doc.IsObject() || !doc.HasMember("nodeTree")) {
        return nullptr;
    }
    pScene scene = Scene::GenerateObject();
    pEntity scene_entity = WalkSceneJson(doc["nodeTree"]);

    SkeletonManager::Instance().CreateSkeletonEntities(scene->root);
    s_Traverse(scene_entity,
               [](const std::shared_ptr<Entity>& ptr) {
                   auto c = ptr->GetComponent(kMeshFilter);
                   if (c) {
                       auto mfc = std::dynamic_pointer_cast<MeshFilterComponent>(c);
                       if (mfc->GetMesh()) {
                           // try link bones
                           mfc->GetMesh()->LinkBones(ptr);
                       }
                   }
                   c = ptr->GetComponent(kAnimator);
                   if (c) {
                       auto ac = std::dynamic_pointer_cast<AnimatorComponent>(c);
                       for (auto& m : ac->GetAnimatorClips()) {
                           // try link entities
                           m->LinkToObjectsAndProperties(ptr);
                       }
                   }
               },
               nullptr);

    scene->AppendEntityToRoot(scene_entity);
    scene->UpdateGlobalSettings();
    scene->Update();
    return scene;
}

pEntity GAS2Loader::WalkHierarchyRecursive(const rapidjson::Value& v, const pEntity& parent) {
    pEntity entity = EntityFactory::Instance().CreateEntity(parent);
    json::GetMember(v, "name", entity->name);
    json::GetMember(v, "skeletonName", entity->skeleton_name);
    json::GetMember(v, "uniqueID", entity->unique_id);
    entity->has_mb_props = json::GetMember(v, "MB_PROPS", entity->mb_props);
    if (entity->has_mb_props) {
        entity->mb_props.UpdateMatrix();
    }
    entity->has_max_props = json::GetMember(v, "MAX_PROPS", entity->max_props);
    if (entity->has_max_props) {
        entity->max_props.UpdateMatrix();
    }

    if (entity->skeleton_name != "eNone") {
        SkeletonManager::Instance().AppendBone(entity);
    }

    std::vector<float> double_vec;
    json::GetMember(v, "translation", double_vec);
    if (double_vec.size() >= 3) {
        entity->SetLocalTranslation(glm::vec3(double_vec[0], double_vec[1], double_vec[2]));
    }
    double_vec.clear();
    json::GetMember(v, "rotation", double_vec);
    if (double_vec.size() >= 3) {
        entity->SetLocalRotationByEuler(
            glm::vec3(
                // convert deg 2 rad
                glm::radians(double_vec[0]), glm::radians(double_vec[1]),
                glm::radians(double_vec[2])),
            kEulerMode_ZYX);
    }
    double_vec.clear();
    json::GetMember(v, "scaling", double_vec);
    if (double_vec.size() >= 3) {
        entity->SetLocalScale(glm::vec3(double_vec[0], double_vec[1], double_vec[2]));
    }
    double_vec.clear();
    // components
    {
        auto it = v.FindMember("components");
        if (it != v.MemberEnd() && it->value.GetType() == rapidjson::kObjectType) {
            for (auto component_it = it->value.MemberBegin(); component_it != it->value.MemberEnd();
                 ++component_it) {
                std::string component_type = json::ToString(component_it->name);
                const rapidjson::Value& component_value = component_it->value;

                auto loader = ComponentLoaderFactory::Instance().CreateLoader(component_type,
                                                                              component_value);
                if (loader == nullptr) {
                    LOG_ERROR("can not load component type: %s", component_type.c_str());
                } else {
                    pComponent loaded = loader->Load(hint_pathes);
                    entity->AddComponent(loaded);
                }
            }
        }
    }
    // children
    {
        auto it = v.FindMember("children");
        if (it != v.MemberEnd() && it->value.GetType() == rapidjson::kArrayType) {
            for (const auto& child : it->value.GetArray()) {
                WalkHierarchyRecursive(child, entity);
            }
        }
    }

    return entity;
}