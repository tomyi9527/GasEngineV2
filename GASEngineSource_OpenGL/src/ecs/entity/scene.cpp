#include "scene.h"
#include <stack>
#include "data_types/keyframe_animator_loader.h"
#include "data_types/material/compound_material.h"
#include "ecs/component/animator_component.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component/mesh_renderer_component.h"
#include "ecs/entity_factory.h"
#include "opengl/global_resource.h"

// TODO(beanpliu): support finding entity
void Scene::FindObjectByPath(const std::string& path) {
    // 1. find from root:  "/1/2/3"
}
void Scene::FindObjectByID(uint64_t id) {
    // 1. find from root:  12345
}
void Scene::FindObjectByName(const std::string& name) {
    // 1. find from root:  "3"
}
void Scene::FindComponents(kComponentType type) {
    // return vector
}
void Scene::Cull() {
    if (cameras.empty()) {
        return;
    }
    // add renderable into camera renderlist
    for (auto& camera : cameras) {
        camera->ClearRenderables();
        std::stack<pEntity> stack;
        stack.push(root);
        while (!stack.empty()) {
            auto entity = stack.top();
            stack.pop();
            if (!entity->enable) {
                continue;
            }
            pComponent ptr;
            ptr = entity->GetComponent(kMeshFilter);
            if (ptr != nullptr) {
                auto mesh_filter = std::dynamic_pointer_cast<MeshFilterComponent>(ptr);
                auto mesh = mesh_filter->GetMesh();
                if (mesh) {
                    glm::vec3 ndc = DecomposeTranslation(entity->GetWorldMatrix());
                    ndc = camera->GetViewProjectionMatrix() * glm::vec4(ndc, 1.0);
                    // try take out material
                    ptr = entity->GetComponent(kMeshRenderer);
                    if (ptr == nullptr) {
                        continue;
                    }
                    auto mesh_renderer = std::dynamic_pointer_cast<MeshRendererComponent>(ptr);
                    // try take out submesh
                    for (size_t i = 0; i < mesh->submesh.size(); ++i) {
                        const SUBMESH& submesh = mesh->submesh[i];
                        int material_index_from_submesh = -1;
                        // int material_index_from_submesh = submesh.material_index;
                        size_t material_index = i;
                        if (material_index_from_submesh >= 0) {
                            material_index = material_index_from_submesh;
                        }
                        std::shared_ptr<Material> material;
                        if (mesh_renderer->GetMaterials().size() > material_index) {
                            material = mesh_renderer->GetMaterials()[material_index];
                        }
                        if (material != nullptr) {
                            if (material->GetType() == kHotspotMaterial) {
                                camera->SetHotSpotItem(mesh, material, submesh);
                            } else if (material->GetType() == kCompoundMaterial) {
                                material = std::dynamic_pointer_cast<CompoundMaterial>(material)
                                               ->GetActiveMaterial();
                            }
                            if (material->IsVisible()) {
                                camera->AppendRenderables(
                                    mesh, material, submesh, environmental_light, punctual_lights,
                                    directional_lights, point_lights, spot_lights,
                                    entity->GetWorldMatrix(), ndc.z, entity->is_helper);
                            }
                        }
                    }
                }
            }
            for (const auto& child : entity->children) {
                stack.push(child);
            }
        }

        break;  // process only first camera now.
    }
}

void Scene::UpdateGlobalSettings() {
    // update animation
    if (animators.empty()) {
        AppGlobalResource::Instance().SetEnableMorphAnimation(false);
        AppGlobalResource::Instance().SetEnableSkinning(false);
    } else {
        AppGlobalResource::Instance().SetEnableMorphAnimation(true);
        AppGlobalResource::Instance().SetEnableSkinning(true);
    }
}

void Scene::Update(float delta) {
    for (auto& m : active_clips) {
        m->Update(delta);
    }

    if (!animators.empty()) {
        auto animator = GetActiveAnimator();
        auto clip = GetActiveAnimation();
        if (clip && animator) {
            clip->speed = animator_play_speed;
            // loop mode
            if (clip->clip_ended) {
                if (animator_loop_mode == kAnimation_Loop) {
                    animator->PlayNext(clip);
                } else if (animator_loop_mode == kAnimation_Repeat) {
                    clip->SetProgress(0.0f);
                } else if (animator_loop_mode == kAnimation_Shuffle) {
                    animator->PlayRandom();
                }
            }
            // play mode
            clip->enable = animator_play_mode == kAnimation_Play;
            if (animator_play_mode == kAnimation_Stop) {
                clip->SetProgress(0.0f);
            }
            // enable clamp
            clip->enable_clamp = animation_enable_clamp;
        }
    }

    // clear current variables
    this->active_camera_index = -1;
    this->cameras.clear();
    this->directional_lights.clear();
    this->environmental_light = nullptr;
    this->punctual_lights.clear();
    this->spot_lights.clear();
    this->animators.clear();
    this->active_clips.clear();
    // update Entity Tree
    if (root != nullptr) {
        root->Update();
    }
    // update camera matrix
    for (auto& m : cameras) {
        m->UpdateWorldMatrix();
        m->UpdateProjectionMatrix();
        m->UpdateViewProjectionMatrix();
    }
}

void Scene::PickObject(int x, int y) {
    // find intersection on each entity-mesh_filter_component-triangle
}

void Scene::SetShadingMode(kShadingMode mode) {}

std::shared_ptr<AnimatorComponent> Scene::GetActiveAnimator() const {
    if (animators.empty()) {
        return nullptr;
    }
    return (*animators.begin());
}

std::shared_ptr<KeyframeAnimation> Scene::GetActiveAnimation() const {
    auto c = GetActiveAnimator();
    if (c) {
        return c->GetActiveAnimatorClip();
    } else {
        return nullptr;
    }
}

float Scene::GetActiveAnimationProgress() const {
    auto active_clip = GetActiveAnimation();
    if (!active_clip) {
        return 0.0f;
    } else {
        return active_clip->GetProgress();
    }
}

void Scene::SetActiveAnimationProgress(float progress) {
    auto active_clip = GetActiveAnimation();
    if (active_clip) {
        active_clip->SetProgress(std::clamp(progress, 0.0f, 1.0f));
    }
}

float Scene::GetActiveAnimationTotalDuration() const {
    auto active_animator = GetActiveAnimator();
    float ret = 0;
    if (active_animator) {
        for (const auto& m : active_animator->GetAnimatorClips()) {
            ret += m->Duration();
        }
    }
    return ret;
}
