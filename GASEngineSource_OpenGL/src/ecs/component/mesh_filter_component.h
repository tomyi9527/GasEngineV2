#pragma once
#include <memory>
#include "data_types/mesh_loader.h"
#include "ecs/component_factory.h"
#include "ecs/entity_factory.h"
#include "utils/bbox.h"

class MeshFilterComponent : public Component {
 public:
    static std::shared_ptr<Component> GenerateComponent() {
        return std::make_shared<MeshFilterComponent>();
    }

    MeshFilterComponent() : Component(kMeshFilter) {}

    std::shared_ptr<OBJECT> GetMesh() const { return mesh_; }
    const std::shared_ptr<OBJECT>& GetMesh() { return mesh_; }
    void SetMesh(const std::shared_ptr<OBJECT>& mesh) { mesh_ = mesh; }

    const BBOX<3>& GetBBox() const { return bbox_; }
    BBOX<3>& GetBBox() { return bbox_; }
    void SetBBox(const BBOX<3>& bbox) { bbox_ = bbox; }

    void LoadMesh(std::istream& s);
    void UpdateBBox();

 private:
    std::shared_ptr<OBJECT> mesh_;
    BBOX<3> bbox_;
};