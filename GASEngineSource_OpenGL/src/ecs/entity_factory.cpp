#include "entity_factory.h"
#include <stack>
#include "ecs/component/animator_component.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/entity/scene.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "opengl/global_resource.h"
#include "utils/encoding_conv.h"

bool MB_PROPS::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    std::vector<float> vals;
    ret = json::GetMember(v, "ScalingPivot", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) scaling_pivot[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "ScalingOffset", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) scaling_offset[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "RotationPivot", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) rotation_pivot[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "RotationOffset", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) rotation_offset[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "PreRotation", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) pre_rotation[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "PostRotation", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) post_rotation[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "RotationOrder", rotation_order) && ret;
    ret = json::GetMember(v, "InheritType", inherit_type) && ret;
    ret = json::GetMember(v, "Visibility", visibility) && ret;
    ret = json::GetMember(v, "VisibilityInheritance", visibility_inheritance) && ret;
    return true;
}

void MB_PROPS::AddToJson(json::JsonDoc& doc, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    std::vector<float> vals;
    vals.assign(&(scaling_pivot[0]), &(scaling_pivot[0]) + 3);
    doc.AddMemberTo(v, "ScalingPivot", vals);
    vals.assign(&(scaling_offset[0]), &(scaling_offset[0]) + 3);
    doc.AddMemberTo(v, "ScalingOffset", vals);
    vals.assign(&(rotation_pivot[0]), &(rotation_pivot[0]) + 3);
    doc.AddMemberTo(v, "RotationPivot", vals);
    vals.assign(&(rotation_offset[0]), &(rotation_offset[0]) + 3);
    doc.AddMemberTo(v, "RotationOffset", vals);
    vals.assign(&(pre_rotation[0]), &(pre_rotation[0]) + 3);
    doc.AddMemberTo(v, "PreRotation", vals);
    vals.assign(&(post_rotation[0]), &(post_rotation[0]) + 3);
    doc.AddMemberTo(v, "PostRotation", vals);

    doc.AddMemberTo(v, "RotationOrder", rotation_order);
    doc.AddMemberTo(v, "InheritType", inherit_type);
    doc.AddMemberTo(v, "Visibility", visibility);
    doc.AddMemberTo(v, "VisibilityInheritance", visibility_inheritance);
}

void MB_PROPS::UpdateMatrix() {
    const glm::mat4 identity = glm::mat4(1.0f);
    kEulerMode mode = kEulerMode_ZYX;
    // scale
    scaling_pivot_matrix = glm::translate(identity, scaling_pivot);
    scaling_pivot_inverse_matrix = glm::translate(identity, -scaling_pivot);
    scaling_offset_matrix = glm::translate(identity, scaling_offset);
    // rotation
    rotation_pivot_matrix = glm::translate(identity, rotation_pivot);
    rotation_pivot_inverse_matrix = glm::translate(identity, -rotation_pivot);
    rotation_offset_matrix = glm::translate(identity, rotation_offset);
    glm::vec3 rad_pre_rotation = glm::radians(pre_rotation);
    pre_rotation_matrix = MatFromEulerXYZ(rad_pre_rotation, mode);
    glm::vec3 rad_post_rotation = glm::radians(post_rotation);
    post_rotation_inverse_matrix = glm::inverse(MatFromEulerXYZ(rad_post_rotation, mode));
    // premult
    pre_multi_0 = post_rotation_inverse_matrix * rotation_pivot_inverse_matrix *
                  scaling_offset_matrix * scaling_pivot_matrix;
    pre_multi_1 = rotation_offset_matrix * rotation_pivot_matrix * pre_rotation_matrix;
}

bool MAX_PROPS::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    std::vector<float> vals;
    ret = json::GetMember(v, "ScalingAxis", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 4; ++i) scaling_axis[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "OffsetRotation", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 4; ++i) offset_rotation[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "OffsetTranslation", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) offset_translation[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "OffsetScaling", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 3; ++i) offset_scaling[i] = vals[i];
    vals.clear();
    ret = json::GetMember(v, "OffsetScalingAxis", vals) && ret;
    for (size_t i = 0; i < vals.size() && i < 4; ++i) offset_scaling_axis[i] = vals[i];
    vals.clear();
    return true;
}

void MAX_PROPS::AddToJson(json::JsonDoc& doc, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    std::vector<float> vals;
    vals.assign(&(scaling_axis[0]), &(scaling_axis[0]) + 4);
    doc.AddMemberTo(v, "ScalingAxis", vals);
    vals.assign(&(offset_rotation[0]), &(offset_rotation[0]) + 4);
    doc.AddMemberTo(v, "OffsetRotation", vals);
    vals.assign(&(offset_translation[0]), &(offset_translation[0]) + 3);
    doc.AddMemberTo(v, "OffsetTranslation", vals);
    vals.assign(&(offset_scaling[0]), &(offset_scaling[0]) + 3);
    doc.AddMemberTo(v, "OffsetScaling", vals);
    vals.assign(&(offset_scaling_axis[0]), &(offset_scaling_axis[0]) + 4);
    doc.AddMemberTo(v, "OffsetScalingAxis", vals);
}

inline glm::mat4 Vec4AsQuatToMat(const glm::vec4& v) {
    Rotation rotation;

    glm::quat quat;
    quat = glm::make_quat(glm::value_ptr(v));
    rotation.FromQuat(quat);

    return rotation.AsMat();
}

void MAX_PROPS::UpdateMatrix() {
    const glm::mat4 identity = glm::mat4(1.0f);

    // sa
    scaling_axis_matrix = Vec4AsQuatToMat(scaling_axis);
    scaling_axis_inverse_matrix = glm::inverse(scaling_axis_matrix);

    // ot 
    offset_translation_matrix = glm::translate(identity, offset_translation);

    // or
    offset_rotation_matrix = Vec4AsQuatToMat(offset_rotation);

    // os, osa
    offset_scaling_matrix = glm::scale(identity, offset_scaling);
    offset_scaling_axis_matrix = Vec4AsQuatToMat(offset_scaling_axis);
    offset_scaling_axis_inverse_matrix = glm::inverse(offset_scaling_axis_matrix);

    // offset_matrix:
    // ot * or * osa^-1 * os * osa
    post_multi_0 = offset_translation_matrix * offset_rotation_matrix *
                   offset_scaling_axis_inverse_matrix * offset_scaling_matrix *
                   offset_scaling_axis_matrix;

    // hierarchy_matrix:
    // t * r * sa^-1 * s * sa
}

std::shared_ptr<Entity> Entity::FindChildEntityByID(int64_t id) {
    std::stack<pEntity> bfs;
    bfs.push(GetPtr());

    while (!bfs.empty()) {
        auto entity = bfs.top();
        bfs.pop();
        if (entity->unique_id == id) {
            return entity;
        }
        for (const auto& component : entity->components) {
            if (component.second->GetID() == id) {
                return entity;
            }
        }
        for (const auto& child : entity->children) {
            bfs.push(child);
        }
    }
    return nullptr;
}

std::shared_ptr<Entity> Entity::FindChildEntityByName(const std::string& name) {
    std::stack<pEntity> bfs;
    bfs.push(GetPtr());

    while (!bfs.empty()) {
        auto entity = bfs.top();
        bfs.pop();
        if (entity->name == name) {
            return entity;
        }
        for (const auto& child : entity->children) {
            bfs.push(child);
        }
    }
    return nullptr;
}

bool Entity::HasEntityInParent(const std::shared_ptr<Entity>& in_child) {
    auto parent_ptr = parent;
    while (parent_ptr != nullptr) {
        if (parent_ptr == in_child) {
            return true;
        }
        parent_ptr = parent_ptr->parent;
        if (parent_ptr == parent) {
            LOG_ERROR("Loop detected");
            break;
        }
    }
    return false;
}

bool Entity::HasEntityInChild(const std::shared_ptr<Entity>& in_child) {
    for (auto it = children.begin(); it < children.end(); ++it) {
        if (in_child == *it) {
            return true;
        }
    }
    return false;
}
void Entity::RemoveChild(const std::shared_ptr<Entity>& in_child) {
    if (in_child == nullptr || in_child->parent != GetPtr()) {
        return;
    }
    in_child->SetParent(nullptr);
    assert(!HasEntityInChild(in_child));
    return;
}
void Entity::AddChild(const std::shared_ptr<Entity>& in_child) {
    if (in_child == nullptr) {
        return;
    }
    in_child->SetParent(GetPtr());
    assert(HasEntityInChild(in_child));
    return;
}
void Entity::SetParent(const std::shared_ptr<Entity>& in_parent) {
    if (parent != nullptr) {
        parent->RemoveChildInner(GetPtr());
    }
    if (in_parent != nullptr) {
        in_parent->AddChildInner(GetPtr());
    }
    SetParentInner(in_parent);
    if (parent != nullptr) {
        assert(parent->HasEntityInChild(GetPtr()));
    }
}
void Entity::SetScene(const std::shared_ptr<Scene>& in_scene_root) { scene = in_scene_root; }
void Entity::Reset() {
    scene.reset();
    parent.reset();
    for (auto& m : children) {
        // m->Reset();
        EntityFactory::Instance().Destroy(m);
    }
    children.clear();
    components.clear();
}

bool Entity::HasComponent(kComponentType type) const {
    auto it = components.find(type);
    return it != components.end();
}
void Entity::AddComponent(const std::shared_ptr<Component>& ptr) {
    if (ptr == nullptr) {
        return;
    }
    auto it = components.find(ptr->GetType());
    if (it != components.end()) {
        LOG_ERROR("replace component %s", ptr->GetTypeStr().c_str());
        it->second = ptr;
    } else {
        components.emplace(ptr->GetType(), ptr);
    }
    assert(HasComponent(ptr->GetType()));
    ptr->SetParent(GetPtr());
}
std::shared_ptr<Component> Entity::GetComponent(kComponentType type) const {
    auto it = components.find(type);
    if (it == components.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}
void Entity::RemoveComponent(kComponentType type) {
    auto it = components.find(type);
    if (it != components.end()) {
        components.erase(it);
    }
    assert(!HasComponent(type));
}

// 参考 https://www.zhihu.com/question/280630276/answer/415783268
void Entity::Print(std::ostream& o, int indent) {
    // o.imbue(std::locale());
    static int vec_left[100] = {0};
    if (indent > 0) {
        for (int i = 0; i < indent - 1; ++i) {
            if (vec_left[i]) {
                o << "|   ";
            } else {
                o << "    ";
            }
        }
        if (vec_left[indent - 1]) {
            o << "+-- ";
        } else {
            o << "`-- ";
        }
    }
    o << "o ";
    std::string components_str;
    for (const auto& item : components) {
        components_str += item.second->GetTypeStr();
        components_str += ", ";
    }
    o << "\"" << name << "\"";
    if (!components_str.empty()) {
        components_str.pop_back();
        components_str.pop_back();
        o << "  components: " << components_str;
    } else {
        o << "  no component";
    }
    if (children.empty()) {
        o << "  no children";
    } else {
        o << "  " << children.size() << " children";
    }
    o << std::endl;
    vec_left[indent] = 1;
    int count = 0;
    for (auto& m : children) {
        if (++count == children.size()) vec_left[indent] = 0;
        m->Print(o, indent + 1);
    }
    vec_left[indent] = 0;
}

void Entity::UpdateLocalMatrix(bool use_quat_for_rotation) {
    if (use_fixed_local_matrix) {
        return;
    }
    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
    if (use_quat_for_rotation) {
        SetLocalRotationByQuat(rotation_quaternion);
    } else {
        SetLocalRotationByEuler(rotation_euler, rotation_euler_mode);
    }
    glm::mat4 R = rotation_matrix;
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    if (has_max_props) {
        matrix_local = T * R * max_props.scaling_axis_inverse_matrix * S *
                       max_props.scaling_axis_matrix;
    } else if (has_mb_props && AppGlobalResource::Instance().EnableMotionBuilderParameters()) {
        matrix_local = T * mb_props.pre_multi_1 * R * mb_props.pre_multi_0 * S *
                       mb_props.scaling_pivot_inverse_matrix;
    } else {
        matrix_local = T * R * S;
    }
}
void Entity::UpdateWorldMatrix() {
    // update world transformation
    if (parent != nullptr) {
        matrix_world = parent->matrix_world * matrix_local;
    } else {
        matrix_world = matrix_local;
    }
}
void Entity::UpdateMaxMatrix() {
    if (has_max_props){
        matrix_local = matrix_local * max_props.post_multi_0;
        UpdateWorldMatrix();
    }
}
void Entity::Update() {
    AppGlobalResource::Instance().SetEnableMotionBuilderParameters(true);
    if (AppGlobalResource::Instance().EnableMotionBuilderParameters()) {
        // motion builder setting
        if (mb_props.visibility_inheritance == true && parent != nullptr) {
            mb_props.visibility = parent->mb_props.visibility;
            enable = enable && parent->mb_props.visibility;
        }
    }

    // update local transformation
    auto scene_ptr = scene.lock();
    if (!scene_ptr) {
        LOG_ERROR("scene expired.");
    }
    if (enable && scene_ptr) {
        UpdateLocalMatrix();
        UpdateWorldMatrix();

        // add components to scene
        pComponent animator = GetComponent(kAnimator);
        if (animator != nullptr) {
            auto animator_component = std::dynamic_pointer_cast<AnimatorComponent>(animator);
            scene_ptr->AddAnimator(animator_component);
            auto clip = animator_component->GetActiveAnimatorClip();
            if (clip != nullptr) {
                scene_ptr->AddActiveAnimationClip(clip);
            }
        }
        // camera
        pComponent camera = GetComponent(kCamera);
        if (camera != nullptr) {
            scene_ptr->AddCamera(std::dynamic_pointer_cast<CameraComponent>(camera));
        }
        // lights
        pComponent environmental_light = GetComponent(kEnvironmentalLight);
        if (environmental_light != nullptr) {
            scene_ptr->SetEnvironmentLight(
                std::dynamic_pointer_cast<EnvironmentalLightComponent>(environmental_light));
        }
        // pComponent punctual_light = GetComponent(kPunctualLight);
        // if (punctual_light != nullptr) {
        //    scene_ptr->AddPunctualLight(std::dynamic_pointer_cast<PunctualLightComponent>(punctual_light));
        //}
        // pComponent directional_light = GetComponent(kDirectionalLight);
        // if (punctual_light != nullptr) {
        //    scene_ptr->AddDirectionalLight(std::dynamic_pointer_cast<DirectionalLightComponent>(directional_light));
        //}
        // pComponent point_light = GetComponent(kPointLight);
        // if (point_light != nullptr) {
        //    //
        //    scene_ptr->AddPointLight(std::dynamic_pointer_cast<PointLightComponent>(point_light));
        //    LOG_ERROR("point_light component is not supported now.");
        //}
        // pComponent spot_light = GetComponent(kSpotLight);
        // if (spot_light != nullptr) {
        //    scene_ptr->AddSpotLight(std::dynamic_pointer_cast<SpotLightComponent>(spot_light));
        //}

        ///////////// Pre Operations above
        // update children
        for (auto& m : children) {
            m->Update();
        }
        ///////////// Post Operations below
        // update 3ds max offset matrix
        UpdateMaxMatrix();

        // update bbox
        bbox.Reset();
        // 1. from mesh_filter
        pComponent mesh_filter = GetComponent(kMeshFilter);
        if (mesh_filter != nullptr) {
            auto ptr = std::dynamic_pointer_cast<MeshFilterComponent>(mesh_filter);
            if (ptr->GetBBox().IsValid()) {
                Box box;
                box.FromAABB(ptr->GetBBox());
                box.ApplyTransform(matrix_world);
                bbox.FromBox(box);
            }
        } else if (components.empty() && scene_ptr->HasAnimator()) {
            // 仅作为非模型父节点时如此处理，否则不设置
            bbox.Reset(GetWorldTranslation());
        }
        // 2. from children
        for (const auto& m : children) {
            bbox.Merge(m->bbox);
        }
    }
}
// 仅修改自身
bool Entity::RemoveChildInner(const std::shared_ptr<Entity>& in_child) {
    for (auto it = children.begin(); it < children.end(); ++it) {
        if (in_child == *it) {
            children.erase(it);
            return true;
        }
    }
    return false;
}
void Entity::AddChildInner(const std::shared_ptr<Entity>& in_child) {
    if (HasEntityInChild(in_child) || HasEntityInParent(in_child)) {
        return;
    }
    children.push_back(in_child);
    return;
}
void Entity::SetParentInner(const std::shared_ptr<Entity>& in_parent) {
    parent = in_parent;
    if (in_parent) {
        scene = in_parent->scene;
    } else {
        scene.reset();
    }
}

// properties
void Entity::SetLocalTranslation(const glm::vec3& in) {
    use_fixed_local_matrix = false;
    translation = in;
}
void Entity::SetLocalRotationByEuler(const glm::vec3& in, kEulerMode in_mode) {
    rotation_euler_mode = in_mode;
    SetLocalRotationMatrix(MatFromEulerXYZ(in, in_mode));
}
void Entity::SetLocalRotationByQuat(const glm::quat& quat) {
    SetLocalRotationMatrix(glm::toMat4(quat));
}
void Entity::SetLocalRotationMatrix(const glm::mat4& mat) {
    use_fixed_local_matrix = false;
    rotation_matrix = mat;
    rotation_quaternion = glm::toQuat(rotation_matrix);  // quat
    auto e = EulerFromMatrix(rotation_matrix, rotation_euler_mode);
    rotation_euler = e.xyz;
}
void Entity::SetLocalScale(const glm::vec3& in) {
    use_fixed_local_matrix = false;
    scale = in;
}

const glm::vec3& Entity::GetLocalScale() const { return scale; }
glm::quat Entity::GetLocalQuaternion() const { return rotation_quaternion; }
std::pair<glm::vec3, kEulerMode> Entity::GetLocalRotation() const {
    return {rotation_euler, rotation_euler_mode};
}
const glm::vec3& Entity::GetLocalTranslation() const { return translation; }
const glm::mat4& Entity::GetLocalMatrix() const { return matrix_local; }

const glm::mat4& Entity::GetWorldMatrix() const { return matrix_world; }
glm::mat4 Entity::GetWorldRotation() const { return glm::toMat4(GetWorldQuaternion()); }
glm::quat Entity::GetWorldQuaternion() const {
    glm::quat world_quat = rotation_quaternion;
    pEntity current_parent = parent;
    while (current_parent != nullptr) {
        world_quat *= ScaleMulQuat(current_parent->scale, current_parent->rotation_quaternion);
        current_parent = current_parent->parent;
    }
    return world_quat;
}
glm::vec3 Entity::GetWorldTranslation() const {
    glm::vec3 ret = translation;
    if (parent != nullptr) {
        ret = parent->matrix_world * glm::vec4(ret, 1.0);
    }
    return ret;
}
glm::vec3 Entity::GetWorldScale() const { return DecomposeScale(matrix_local); }