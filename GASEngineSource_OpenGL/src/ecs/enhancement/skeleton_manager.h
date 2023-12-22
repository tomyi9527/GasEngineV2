#pragma once
#include <functional>
#include <vector>
#include <set>
#include "ecs/entity_factory.h"
#include "ecs/entity/scene.h"
#include "ecs/component_factory.h"

class SkeletonManager {
 public:
    static SkeletonManager& Instance() { 
        static SkeletonManager instance;
        return instance;
    }

    void AppendBones(const std::vector<pEntity>& bone_list);
    void AppendBone(const pEntity& bone_entity);
    void CreateSkeletonEntities(const pEntity& parent = nullptr);
    pEntity GetEntity() const;
    void Update();

 protected:
    void UpdateMinMax();
    void CreateHelperEntity(const pEntity& parent = nullptr);
    void ApplyTransformToJoint(const pEntity& src, const pEntity& dest);
    void ApplyTransformToBone(const pEntity& src, const pEntity& dest);

 protected:
    pEntity CreateJoint(const pEntity& target);
    pEntity CreateBone(const pEntity& child, const pEntity& parent);
    std::set<pEntity> bone_set;
    std::vector<pEntity> children;
    std::vector<std::function<void()>> update_system;
    pEntity manager_entity;
    float scale = 1.0f;
    float min_scale = std::numeric_limits<float>::max();
    float max_scale = 0.0f;
};