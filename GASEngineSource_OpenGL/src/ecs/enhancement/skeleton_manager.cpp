#include "skeleton_manager.h"
#include "data_types/geometry.h"
#include "data_types/material_factory.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component/mesh_renderer_component.h"

pEntity SkeletonManager::GetEntity() const { return manager_entity; }

void SkeletonManager::Update() {
    if (min_scale > max_scale) {
        UpdateMinMax();
    }
    if (max_scale >= min_scale) {
        for (const auto& m : update_system) m();
    }
}

void SkeletonManager::UpdateMinMax() {
    // distance range is [min_scaling, max_scaling]
    min_scale = std::numeric_limits<float>::max();
    max_scale = 0;
    for (const auto& m : bone_set) {
        if (m->parent != nullptr && bone_set.count(m->parent) != 0) {
            float current_distance =
                glm::l2Norm(m->GetWorldTranslation() - m->parent->GetWorldTranslation());
            if (current_distance > 0.0f) {
                min_scale = std::min(current_distance, min_scale);
                max_scale = std::max(current_distance, max_scale);
            }
        }
    }
}

void SkeletonManager::CreateHelperEntity(const pEntity& parent) {
    if (manager_entity != nullptr) {
        manager_entity->parent->RemoveChild(manager_entity);
    }
    manager_entity = EntityFactory::Instance().CreateEntity(parent);
    manager_entity->enable = true;
    manager_entity->name = "skeleton_helper";
    manager_entity->is_helper = true;
}

void SkeletonManager::AppendBones(const std::vector<pEntity>& bone_list) {
    for (const auto& m : bone_list) {
        if (bone_set.count(m) == 0) {
            bone_set.emplace(m);
        }
    }
}

void SkeletonManager::AppendBone(const pEntity& bone_entity) {
    if (bone_set.count(bone_entity) == 0) {
        bone_set.emplace(bone_entity);
    }
}

void SkeletonManager::ApplyTransformToBone(const pEntity& src, const pEntity& dest) {
    assert(src->parent != nullptr);
    glm::vec3 vec = src->GetWorldTranslation() - src->parent->GetWorldTranslation();
    float distance = glm::l2Norm(vec);
    float xy_scale = scale * std::max(distance, min_scale) / 15.0f;
    glm::mat4 matrix_model_scale = glm::scale(glm::mat4(1.0f), {xy_scale, xy_scale, distance});

    glm::vec3 direction = glm::normalize(vec);
    glm::mat4 matrix_model_rotate(1.0f);
    if (std::abs(direction.z) < 0.99999) {
        glm::vec3 rotate_axis = glm::normalize(glm::vec3(-direction.y, direction.x, 0));
        float cos_theta_d_2 = std::sqrt((1 + direction.z) / 2);
        float sin_theta_d_2 = std::sqrt(1 - cos_theta_d_2 * cos_theta_d_2);
        glm::quat quat(cos_theta_d_2, sin_theta_d_2 * rotate_axis);
        Rotation r;
        r.FromQuat(quat);
        matrix_model_rotate = r.AsMat();
        // check should equals to pos;
        // glm::vec3 check = matrix_model_rotate * glm::vec4{0, 0, distance, 1};
        // auto diff = glm::abs(check - pos);
        // auto diff_val = diff.x + diff.y + diff.z;
    } else if (direction.z < 0) {
        matrix_model_rotate[0][0] = -1.0f;
        matrix_model_rotate[2][2] = -1.0f;
    }

    glm::mat4 matrix_world = glm::translate(glm::mat4(1.0f), src->parent->GetWorldTranslation());
    dest->SetLocalMatrix(matrix_world * matrix_model_rotate * matrix_model_scale);
    dest->UpdateWorldMatrix();
}

void SkeletonManager::ApplyTransformToJoint(const pEntity& src, const pEntity& dest) {
    glm::vec3 vec = src->GetWorldTranslation();
    if (src->parent != nullptr) {
        vec -= src->parent->GetWorldTranslation();
    }
    float distance = glm::l2Norm(vec);

    float radius = scale * std::clamp(distance, min_scale, (max_scale + min_scale) / 2.0f) / 12.0f;
    glm::mat4 matrix_model = glm::scale(glm::mat4(1.0f), {radius, radius, radius});
    glm::mat4 matrix_world = glm::translate(glm::mat4(1.0f), src->GetWorldTranslation());
    dest->SetLocalMatrix(matrix_world * matrix_model);
    dest->UpdateWorldMatrix();
}

void SkeletonManager::CreateSkeletonEntities(const pEntity& parent) {
    if (manager_entity == nullptr) {
        CreateHelperEntity(parent);
    }
    for (const auto& m : children) {
        manager_entity->RemoveChild(m);
    }
    children.clear();
    update_system.clear();
    for (const auto& m : bone_set) {
        if (m->parent != nullptr && bone_set.count(m->parent) != 0) {
            pEntity e_bone = CreateBone(m, m->parent);
            children.push_back(e_bone);
            update_system.push_back([m, e_bone, this]() { this->ApplyTransformToBone(m, e_bone); });
        }
        pEntity e_joint = CreateJoint(m);
        children.push_back(e_joint);
        update_system.push_back([m, e_joint, this]() { this->ApplyTransformToJoint(m, e_joint); });
    }
    for (const auto& m : children) {
        manager_entity->AddChild(m);
    }
}

pEntity SkeletonManager::CreateJoint(const pEntity& target) {
    pEntity joint = EntityFactory::Instance().CreateEntity(manager_entity);
    joint->name = "JOINT_" + target->name;
    joint->is_helper = true;
    joint->SetParent(target->parent);

    pComponent c;
    c = ComponentFactory::Instance().Create(kMeshFilter);
    auto mfc = std::dynamic_pointer_cast<MeshFilterComponent>(c);
    // auto sphere = SphereMesh::Generate(radius * scale);
    // sphere->SubmitToOpenGL();
    mfc->SetMesh(SphereMesh::UnitMesh());
    mfc->UpdateBBox();
    joint->AddComponent(mfc);

    c = ComponentFactory::Instance().Create(kMeshRenderer);
    auto mrc = std::dynamic_pointer_cast<MeshRendererComponent>(c);
    auto material = MaterialFactory::Instance().Create(kLambertianMaterial);
    material->LoadAllContent();
    material->SetDepthTest(false);
    mrc->AddMaterial(material);
    joint->AddComponent(mrc);

    return joint;
}

pEntity SkeletonManager::CreateBone(const pEntity& target, const pEntity& parent) {
    pEntity bone = EntityFactory::Instance().CreateEntity(manager_entity);
    bone->name = "BONE_" + target->name;
    bone->is_helper = true;
    bone->SetParent(parent);

    pComponent c;
    c = ComponentFactory::Instance().Create(kMeshFilter);
    auto mfc = std::dynamic_pointer_cast<MeshFilterComponent>(c);
    // auto sphere = PyramidMesh::Generate(target->GetLocalTranslation(), scale);
    // sphere->SubmitToOpenGL();
    mfc->SetMesh(PyramidMesh::UnitMesh());
    mfc->UpdateBBox();
    bone->AddComponent(mfc);

    c = ComponentFactory::Instance().Create(kMeshRenderer);
    auto mrc = std::dynamic_pointer_cast<MeshRendererComponent>(c);
    auto material = MaterialFactory::Instance().Create(kLambertianMaterial);
    material->LoadAllContent();
    material->SetDepthTest(false);
    mrc->AddMaterial(material);
    bone->AddComponent(mrc);

    return bone;
}
