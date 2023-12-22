#include "material_loader.h"
#include "material/blinn_phong_material.h"
#include "material/compound_material.h"
#include "material/dielectric_material.h"
#include "material/electric_material.h"
#include "material/matcap_material.h"
#include "utils/json_maker.h"
#include "utils/logger.h"
#include "utils/resource_manager.h"

pMaterial MaterialLoader::LoadByFile(const std::string& file_path) {
    auto s = resource::ResourceManager::Instance().LoadStream(file_path);
    if (!s || s->fail()) {
        return nullptr;
    } else {
        return LoadByIStream(*s);
    }
}

pMaterial MaterialLoader::LoadByIStream(std::istream& s) {
    std::string input;
    input.assign(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>());

    rapidjson::Document doc;
    doc.Parse(input.data(), input.size());
    if (!doc.IsObject() || !doc.HasMember("type")) {
        return nullptr;
    }

    return LoadMaterial(doc);
}

pMaterial MaterialLoader::LoadMaterial(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return nullptr;
    }
    pMaterial mat = nullptr;

    std::string type;
    json::GetMember(v, "type", type);
    if (type.empty()) {
        return nullptr;
    }

    int type_enum = MaterialFactory::TypeFromString(type);
    if (type_enum == -1) {
        LOG_ERROR("unknown material name: %s", type.c_str());
    } else if (type_enum == kCompoundMaterial) {
        mat = MaterialFactory::Instance().Create(kCompoundMaterial);
        if (mat == nullptr) {
            LOG_ERROR("compoumd material creation failed.");
            return nullptr;
        }
        std::shared_ptr<CompoundMaterial> compound_mat =
            std::dynamic_pointer_cast<CompoundMaterial>(mat);
        std::string active_material;
        json::GetMember(v, "activeMaterial", active_material);
        int active_type = MaterialFactory::TypeFromString(active_material);
        if (active_type == -1) {
            LOG_ERROR("unknown active material name: %s", active_material.c_str());
        }

        for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it) {
            if (MaterialFactory::TypeFromString(json::ToString(it->name)) == -1) {
                continue;
            }
            if (!it->value.IsObject()) {
                continue;
            }
            compound_mat->AddMaterial(this->LoadMaterial(it->value));
        }
        kMaterialType active_type_enum = static_cast<kMaterialType>(active_type);
        compound_mat->SetActiveMaterial(active_type_enum);
        std::string name;
        json::GetMember(v, "name", name);
        mat->SetName(name);
        if (!compound_mat->GetActiveMaterial()) {
            LOG_ERROR("no active material named: %s", active_material.c_str());
            assert(0);
        }
        if (compound_mat->GetActiveMaterial()->GetType() != active_type_enum) {
            LOG_ERROR("%s is not active", active_material.c_str());
        }
    } else if (type_enum == kBlinnPhongMaterial) {
        mat = LoadBlinnPhongMaterial(v);
    } else if (type_enum == kDielectricMaterial) {
        mat = LoadDielectricMaterial(v);
    } else if (type_enum == kElectricMaterial) {
        mat = LoadElectricMaterial(v);
    } else if (type_enum == kMatCapMaterial) {
        mat = LoadMatCapMaterial(v);
    } else {
        LOG_ERROR("loading of material type %s is not supported yet.", type.c_str());
    }

    return mat;
}

pMaterial MaterialLoader::LoadBlinnPhongMaterial(const rapidjson::Value& v) {
    pMaterial mat = MaterialFactory::Instance().Create(kBlinnPhongMaterial);
    std::shared_ptr<BlinnPhongMaterial> blinnphong_mat =
        std::dynamic_pointer_cast<BlinnPhongMaterial>(mat);

    BlinnPhongMaterialData data;
    data.FromJson(v);
    data.path_hint = this->path_hint;
    blinnphong_mat->SetData(data);

    return mat;
}

pMaterial MaterialLoader::LoadDielectricMaterial(const rapidjson::Value& v) {
    pMaterial mat = MaterialFactory::Instance().Create(kDielectricMaterial);
    std::shared_ptr<DielectricMaterial> dielectric_mat =
        std::dynamic_pointer_cast<DielectricMaterial>(mat);

    DielectricMaterialData data;
    data.FromJson(v);
    data.path_hint = this->path_hint;
    dielectric_mat->SetData(data);

    return mat;
}

pMaterial MaterialLoader::LoadElectricMaterial(const rapidjson::Value& v) {
    pMaterial mat = MaterialFactory::Instance().Create(kElectricMaterial);
    std::shared_ptr<ElectricMaterial> electric_mat =
        std::dynamic_pointer_cast<ElectricMaterial>(mat);

    ElectricMaterialData data;
    data.FromJson(v);
    data.path_hint = this->path_hint;
    electric_mat->SetData(data);

    return mat;
}

pMaterial MaterialLoader::LoadMatCapMaterial(const rapidjson::Value& v) {
    pMaterial mat = MaterialFactory::Instance().Create(kMatCapMaterial);
    std::shared_ptr<MatCapMaterial> matcap_mat = std::dynamic_pointer_cast<MatCapMaterial>(mat);

    MatCapMaterialData data;
    data.FromJson(v);
    data.path_hint = this->path_hint;
    matcap_mat->SetData(data);

    return mat;
}

bool TextureMapData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "texture", texture) && ret;
    ret = json::GetMember(v, "pixelChannels", pixelChannels) && ret;
    ret = json::GetMember(v, "wrapModeU", wrapModeU) && ret;
    ret = json::GetMember(v, "wrapModeV", wrapModeV) && ret;
    ret = json::GetMember(v, "minFilter", minFilter) && ret;
    ret = json::GetMember(v, "maxFilter", maxFilter) && ret;
    ret = json::GetMember(v, "uvSwap", uvSwap) && ret;
    ret = json::GetMember(v, "T", T) && ret;
    ret = json::GetMember(v, "R", R) && ret;
    ret = json::GetMember(v, "S", S) && ret;
    ret = json::GetMember(v, "Rp", Rp) && ret;
    ret = json::GetMember(v, "Sp", Sp) && ret;
    return ret;
}

void TextureMapData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "texture", texture);
    js.AddMemberTo(v, "pixelChannels", pixelChannels);
    js.AddMemberTo(v, "wrapModeU", wrapModeU);
    js.AddMemberTo(v, "wrapModeV", wrapModeV);
    js.AddMemberTo(v, "minFilter", minFilter);
    js.AddMemberTo(v, "maxFilter", maxFilter);
    js.AddMemberTo(v, "uvSwap", uvSwap);
    js.AddMemberTo(v, "T", T);
    js.AddMemberTo(v, "R", R);
    js.AddMemberTo(v, "S", S);
    js.AddMemberTo(v, "Rp", Rp);
    js.AddMemberTo(v, "Sp", Sp);
}

bool MaterialPropertyData::FromJson(const rapidjson::Value& v) {
    if (!v.IsObject()) {
        return false;
    }
    bool ret = true;
    ret = json::GetMember(v, "enable", enable) && ret;
    json::GetMember(v, "flipY", flipY);
    json::GetMember(v, "mode", mode);
    json::GetMember(v, "invert", invert);
    json::GetMember(v, "occludeSpecular", occludeSpecular);
    ret = json::GetMember(v, "tint", tint) && ret;
    ret = json::GetMember(v, "factor", factor) && ret;
    ret = json::GetMember(v, "default", default_) && ret;
    ret = json::GetMember(v, "map", map) && ret;
    ret = json::GetMember(v, "srgb", srgb) && ret;
    json::GetMember(v, "multiplicative", multiplicative);
    return ret;
}

void MaterialPropertyData::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) {
        v.SetObject();
    }
    js.AddMemberTo(v, "enable", enable);
    js.AddMemberTo(v, "flipY", flipY);
    js.AddMemberTo(v, "mode", mode);
    js.AddMemberTo(v, "invert", invert);
    js.AddMemberTo(v, "occludeSpecular", occludeSpecular);
    js.AddMemberTo(v, "tint", tint);
    js.AddMemberTo(v, "factor", factor);
    js.AddMemberTo(v, "default", default_);
    js.AddMemberTo(v, "map", map);
    js.AddMemberTo(v, "srgb", srgb);
    js.AddMemberTo(v, "multiplicative", multiplicative);
}