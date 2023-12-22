#pragma once
#include <glm/glm.hpp>
#include "data_types/material/dielectric_material.h"
#include "data_types/material_factory.h"
#include "data_types/material_loader.h"
#include "utils/json_maker.h"

class ElectricMaterialData : public json::JsonSerializeInterface {
 public:
    std::vector<std::string> path_hint;
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int culling = 0;
    std::string name;
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    MaterialPropertyData albedo, specular, roughness, displacement, normal, ao, cavity,
        transparency, emissive;
};

class ElectricMaterial : public DielectricMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<ElectricMaterial>(new ElectricMaterial);
    }

    void SetData(const ElectricMaterialData& data);

 protected:
    ElectricMaterial() : DielectricMaterial(false) {}
};