#pragma once
#include "data_types/material_factory.h"

inline const std::string _empty;

class CompoundMaterial : public Material {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<CompoundMaterial>(new CompoundMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override {
        if (active_material != nullptr) {
            active_material->UpdateUniforms(uniform_values, camera, item, sp);
        }
    }
    uint64_t GenerateVertexShaderKey(const RenderableItem& item) override {
        if (active_material != nullptr) {
            return active_material->GenerateVertexShaderKey(item);
        } else {
            return 0;
        }
    }
    uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override {
        if (active_material != nullptr) {
            return active_material->GenerateFragmentShaderKey(item);
        } else {
            return 0;
        }
    }

    const std::string& GetVertexShaderFile() const override {
        if (active_material != nullptr) {
            return active_material->GetVertexShaderFile();
        } else {
            return _empty;
        }
    }
    const std::string& GetFragmentShaderFile() const override {
        if (active_material != nullptr) {
            return active_material->GetFragmentShaderFile();
        } else {
            return _empty;
        }
    }
    const std::string& GetVertexShaderContent() const override {
        if (active_material != nullptr) {
            return active_material->GetVertexShaderContent();
        } else {
            return _empty;
        }
    }
    const std::string& GetFragmentShaderContent() const override {
        if (active_material != nullptr) {
            return active_material->GetFragmentShaderContent();
        } else {
            return _empty;
        }
    }

    bool IsTransparencyEnabled() const override {
        if (active_material != nullptr) {
            return active_material->IsTransparencyEnabled();
        } else {
            return false;
        }
    }

    void UpdateRenderStates() const override {
        if (active_material != nullptr) {
            active_material->UpdateRenderStates();
        }
    }

    const std::shared_ptr<Material>& GetActiveMaterial() const { return active_material; }

    void AddMaterial(const std::shared_ptr<Material>& material) {
        if (material) {
            materials.emplace(material->GetType(), material);
            if (active_material == nullptr) {
                active_material = material;
            }
        }
    }

    void SetActiveMaterial(kMaterialType type) {
        auto it = materials.find(type);
        if (it != materials.end()) {
            active_material = it->second;
        }
    }

    void ClearMaterials() {
        active_material.reset();
        materials.clear();
    }

    const std::map<kMaterialType, std::shared_ptr<Material>>& GetMaterials() const {
        return materials;
    }

    virtual void SetDepthTest(bool enabled) override {
        if (active_material) active_material->SetDepthTest(enabled);
    }

    virtual void LoadAllContent() override {
        if (active_material) active_material->LoadAllContent();
    }

 protected:
    CompoundMaterial() : Material(kCompoundMaterial) {}

 private:
    std::shared_ptr<Material> active_material;
    std::map<kMaterialType, std::shared_ptr<Material>> materials;
};
