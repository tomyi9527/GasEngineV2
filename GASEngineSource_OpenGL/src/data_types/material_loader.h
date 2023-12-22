#pragma once
#include <istream>
#include <string>
#include "data_types/material_factory.h"
#include "utils/json_maker.h"

class TextureMapData : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string texture;
    int pixelChannels = 0;
    std::string wrapModeU;
    std::string wrapModeV;
    std::string minFilter;
    std::string maxFilter;
    bool uvSwap = false;
    std::vector<double> T;
    std::vector<double> R;
    std::vector<double> S;
    std::vector<double> Rp;
    std::vector<double> Sp;
};

class MaterialPropertyData : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    bool enable = true;
    bool flipY = false;   // normal
    int mode = 0;         // transparency
    bool invert = false;  // transparency, roughness
    std::vector<double> tint;
    double factor = 0.8;
    std::vector<double> default_;
    TextureMapData map;
    bool srgb = true;
    bool multiplicative = false;   // emissive
    bool occludeSpecular = false;  // ao
    double curvature = 0.0;        // matcap
};

class MaterialLoader {
 public:
    static MaterialLoader CreateLoader(const std::vector<std::string>& path_hint) {
        MaterialLoader new_instance(path_hint);
        return new_instance;
    }
    pMaterial LoadByFile(const std::string& file_path);
    pMaterial LoadByIStream(std::istream& s);

 protected:
    MaterialLoader(const std::vector<std::string>& in_path_hint) : path_hint(in_path_hint) {}

    pMaterial LoadMaterial(const rapidjson::Value& v);
    pMaterial LoadBlinnPhongMaterial(const rapidjson::Value& v);
    pMaterial LoadDielectricMaterial(const rapidjson::Value& v);
    pMaterial LoadElectricMaterial(const rapidjson::Value& v);
    pMaterial LoadMatCapMaterial(const rapidjson::Value& v);

    std::vector<std::string> path_hint;
};