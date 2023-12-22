#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include "component_factory.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"
#include "utils/bbox.h"
#include "utils/json_maker.h"
#include "utils/logger.h"
#include "utils/quat_ext.h"

class Scene;

class MB_PROPS : public json::JsonSerializeInterface {
 public:
    bool FromJson(const rapidjson::Value& v) override;
    void AddToJson(json::JsonDoc& doc, rapidjson::Value& v) const override;

 public:
    glm::vec3 scaling_pivot = {0.0, 0.0, 0.0};
    glm::vec3 scaling_offset = {0.0, 0.0, 0.0};
    glm::vec3 rotation_pivot = {0.0, 0.0, 0.0};
    glm::vec3 rotation_offset = {0.0, 0.0, 0.0};
    glm::vec3 pre_rotation = {0.0, 0.0, 0.0};
    glm::vec3 post_rotation = {0.0, 0.0, 0.0};
    std::string rotation_order = "XYZ";
    std::string inherit_type = "Parent(RSrs)";
    double visibility = 1.0;
    bool visibility_inheritance = true;

 public:
    void UpdateMatrix();

    // calculated matrix:
    glm::mat4 scaling_pivot_matrix = glm::mat4(1.0f);
    glm::mat4 scaling_pivot_inverse_matrix = glm::mat4(1.0f);
    glm::mat4 scaling_offset_matrix = glm::mat4(1.0f);
    glm::mat4 rotation_pivot_matrix = glm::mat4(1.0f);
    glm::mat4 rotation_pivot_inverse_matrix = glm::mat4(1.0f);
    glm::mat4 rotation_offset_matrix = glm::mat4(1.0f);
    glm::mat4 pre_rotation_matrix = glm::mat4(1.0f);
    glm::mat4 post_rotation_inverse_matrix = glm::mat4(1.0f);
    glm::mat4 pre_multi_0 = glm::mat4(1.0f);
    glm::mat4 pre_multi_1 = glm::mat4(1.0f);
};

class MAX_PROPS : public json::JsonSerializeInterface {
 public:
    bool FromJson(const rapidjson::Value& v) override;
    void AddToJson(json::JsonDoc& doc, rapidjson::Value& v) const override;

 public:
    glm::vec4 scaling_axis = {0.0, 0.0, 0.0, 1.0};
    glm::vec4 offset_rotation = {0.0, 0.0, 0.0, 1.0};
    glm::vec3 offset_translation = {0.0, 0.0, 0.0};
    glm::vec3 offset_scaling = {0.0, 0.0, 0.0};
    glm::vec4 offset_scaling_axis = {0.0, 0.0, 0.0, 1.0};

 public:
    void UpdateMatrix();

    // calculated matrix:
    glm::mat4 scaling_axis_matrix = glm::mat4(1.0f);
    glm::mat4 scaling_axis_inverse_matrix = glm::mat4(1.0f);
    glm::mat4 offset_rotation_matrix = glm::mat4(1.0f);
    glm::mat4 offset_translation_matrix = glm::mat4(1.0f);
    glm::mat4 offset_scaling_matrix = glm::mat4(1.0f);
    glm::mat4 offset_scaling_axis_matrix = glm::mat4(1.0f);
    glm::mat4 offset_scaling_axis_inverse_matrix = glm::mat4(1.0f);
    glm::mat4 post_multi_0 = glm::mat4(1.0f);
};

class Entity : public std::enable_shared_from_this<Entity> {
 protected:
    Entity()
        : matrix_local(1.0f),
          matrix_world(1.0f),
          translation(0.0f, 0.0f, 0.0f),
          rotation_euler(0.0f, 0.0f, 0.0f),
          rotation_quaternion(glm::vec3(0.0f, 0.0f, 0.0f)),
          rotation_matrix(1.0f),
          scale(1.0f, 1.0f, 1.0f) {}

 public:
    virtual ~Entity() {}
    std::shared_ptr<Entity> GetPtr() { return shared_from_this(); }
    static std::shared_ptr<Entity> GenerateEntity() { return std::shared_ptr<Entity>(new Entity); }

 public:
    // including components
    std::shared_ptr<Entity> FindChildEntityByID(int64_t id);
    std::shared_ptr<Entity> FindChildEntityByName(const std::string& name);
    bool HasEntityInParent(const std::shared_ptr<Entity>& in_child);
    bool HasEntityInChild(const std::shared_ptr<Entity>& in_child);
    void RemoveChild(const std::shared_ptr<Entity>& in_child);
    void AddChild(const std::shared_ptr<Entity>& in_child);
    void SetParent(const std::shared_ptr<Entity>& in_parent);
    void SetScene(const std::shared_ptr<Scene>& in_scene_root);
    void Reset();

    bool HasComponent(kComponentType type) const;
    void AddComponent(const std::shared_ptr<Component>& ptr);
    std::shared_ptr<Component> GetComponent(kComponentType type) const;
    void RemoveComponent(kComponentType type);

    void Print(std::ostream& o, int indent = 0);

 public:
    // properties
    void SetLocalMatrix(const glm::mat4& mat) {
        matrix_local = mat;
        glm::vec3 tmp_skew;
        glm::vec4 tmp_perspective;
        glm::decompose(matrix_local, scale, rotation_quaternion, translation, tmp_skew,
                       tmp_perspective);
        SetLocalRotationByQuat(rotation_quaternion);
        use_fixed_local_matrix = true;
    }
    void SetLocalTranslation(const glm::vec3& in);
    void SetLocalRotationByEuler(const glm::vec3& in, kEulerMode in_mode);
    void SetLocalRotationByQuat(const glm::quat& quat);
    void SetLocalRotationMatrix(const glm::mat4& mat);
    void SetLocalScale(const glm::vec3& in);

    const glm::vec3& GetLocalScale() const;
    glm::quat GetLocalQuaternion() const;
    std::pair<glm::vec3, kEulerMode> GetLocalRotation() const;
    const glm::vec3& GetLocalTranslation() const;
    const glm::mat4& GetLocalMatrix() const;

    const glm::mat4& GetWorldMatrix() const;
    glm::mat4 GetWorldRotation() const;
    glm::quat GetWorldQuaternion() const;
    glm::vec3 GetWorldTranslation() const;
    glm::vec3 GetWorldScale() const;

    // both non recursive
    void UpdateLocalMatrix(bool use_quat_for_rotation = false);
    void UpdateWorldMatrix();
    void UpdateMaxMatrix(); // if max_props, offset_matrix should be applied after all children are updated

    // will update in recursive way, should be called only in scene now.
    void Update();

 protected:
    // 仅修改自身
    bool RemoveChildInner(const std::shared_ptr<Entity>& in_child);
    void AddChildInner(const std::shared_ptr<Entity>& in_child);
    void SetParentInner(const std::shared_ptr<Entity>& in_parent);

 public:
    std::string name;
    std::string skeleton_name;
    int64_t unique_id = -1;
    bool is_mutable = false;
    bool is_helper = false;
    bool enable = true;
    bool has_mb_props = false;
    MB_PROPS mb_props;
    bool has_max_props = false;
    MAX_PROPS max_props;
    std::shared_ptr<Entity> parent = nullptr;
    std::vector<std::shared_ptr<Entity>> children;

    std::map<kComponentType, std::shared_ptr<Component>> components;
    bool use_fixed_local_matrix = false;
    glm::mat4 matrix_local;
    glm::mat4 matrix_world;
    glm::vec3 translation;
    glm::vec3 rotation_euler;
    kEulerMode rotation_euler_mode = kEulerMode_ZYX;
    glm::quat rotation_quaternion;
    glm::mat4 rotation_matrix;
    glm::vec3 scale;

    BBOX<3> bbox;
    std::weak_ptr<Scene> scene;
};

inline void s_Traverse(const std::shared_ptr<Entity>& ptr,
                       const std::function<void(const std::shared_ptr<Entity>& ptr)>& pre,
                       const std::function<void(const std::shared_ptr<Entity>& ptr)>& post) {
    if (ptr == nullptr) {
        return;
    }
    if (pre) pre(ptr);
    for (const auto& m : ptr->children) {
        s_Traverse(m, pre, post);
    }
    if (post) post(ptr);
}

class EntityFactory {
 public:
    static EntityFactory& Instance() {
        static EntityFactory instance;
        return instance;
    }
    std::shared_ptr<Entity> CreateEntity(const std::shared_ptr<Entity>& parent = nullptr) {
        std::shared_ptr<Entity> ret = Entity::GenerateEntity();
        auto result = used_entity_pool_.emplace(ret);
        if (result.second) {
            ret->SetParent(parent);
            return ret;
        } else {
            return nullptr;
        }
    }
    void Destroy(const std::shared_ptr<Entity>& ptr) {
        auto it = used_entity_pool_.find(ptr);
        if (it != used_entity_pool_.end()) {
            (*it)->Reset();
            used_entity_pool_.erase(it);
        }
        return;
    }

 private:
    EntityFactory() {}
    ~EntityFactory() {
        for (auto& m : used_entity_pool_) {
            m->Reset();
        }
        used_entity_pool_.clear();
    }
    std::set<std::shared_ptr<Entity>> used_entity_pool_;
};

using pEntity = std::shared_ptr<Entity>;
