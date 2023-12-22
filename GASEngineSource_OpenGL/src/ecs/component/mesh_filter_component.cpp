#include "mesh_filter_component.h"
#include "ecs/entity/scene.h"
#include "glm/gtc/type_ptr.hpp"

void MeshFilterComponent::LoadMesh(std::istream& s) {
    if (!s || s.eof()) {
        return;
    }
    OBJECT_LOADER loader;
    mesh_ = loader.LoadObject(s);
    if (mesh_) {
        mesh_->SubmitToOpenGL();
    }
    UpdateBBox();
}

void MeshFilterComponent::UpdateBBox() {
    bbox_.Reset();
    if (mesh_) bbox_.FromVertices(mesh_->position);
}