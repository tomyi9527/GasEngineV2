#include "material_factory.h"
#include <filesystem>
#include "data_types/material_loader.h"
#include "ecs/component/mesh_filter_component.h"
#include "material/blinn_phong_material.h"
#include "material/compound_material.h"
#include "material/depth_material.h"
#include "material/dielectric_material.h"
#include "material/electric_material.h"
#include "material/lambertian_material.h"
#include "material/matcap_material.h"
#include "material/pure_color_material.h"
#include "material/shader_key_defination.h"
#include "material/skybox_material.h"
#include "material/uv_layout_material.h"
#include "material/xy_only_pure_color_material.h"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/renderable_item.h"
#include "utils/string_util.h"

std::map<std::string, std::string> SingleMaterial::cache;

// clang-format off
constexpr static const char* names[] = {
    "blinn_phong",
    "dielectric",
    "electric",
    "mat_cap",
    "compound",
    "skybox",  
    "wire_frame",

    "pure_color",
    "depth",
    "hot_spot",
    "uv_layout",
    "lambertian",
    "xy_purecolor"
};


typedef std::shared_ptr<Material> (*GenerateMaterial)();
constexpr static GenerateMaterial generators[] = {
    BlinnPhongMaterial::GenerateMaterial,
    DielectricMaterial::GenerateMaterial,
    ElectricMaterial::GenerateMaterial,
    MatCapMaterial::GenerateMaterial,
    CompoundMaterial::GenerateMaterial,
    SkyBoxMaterial::GenerateMaterial,
    nullptr,

    PureColorMaterial::GenerateMaterial,
    DepthMaterial::GenerateMaterial,
    nullptr,
    UVLayoutMaterial::GenerateMaterial,
    LambertianMaterial::GenerateMaterial,
    XYPureColorMaterial::GenerateMaterial
};

// clang-format on

static_assert((sizeof(names) / sizeof(char*)) == kMaterialTypeCount,
              "kMaterialType should have same length as names");

static_assert((sizeof(generators) / sizeof(GenerateMaterial)) == kMaterialTypeCount,
              "kMaterialType should have same length as generators");

std::string Material::GetTypeStr() const { return names[GetType()]; }

std::shared_ptr<Material> MaterialFactory::Create(kMaterialType type) {
    std::shared_ptr<Material> ret = nullptr;
    if (generators[type] == nullptr) {
        ret = nullptr;
    } else {
        ret = generators[type]();
    }
    // if (ret != nullptr) {
    //     used_materials.emplace(ret);
    // }
    return ret;
}

int MaterialFactory::TypeFromString(std::string type) {
    static std::map<std::string, int> mapper;
    if (mapper.empty()) {
        for (int i = 0; i < kComponentTypeCount; ++i) {
            if (names[i] != nullptr) {
                std::string tmp = names[i];
                mapper.emplace(RemoveSpecialChar(ToLower(tmp), "-_ ."), i);
            }
        }
    }
    RemoveSpecialChar(ToLower(type), "-_ .");
    auto it = mapper.find(type);
    if (it != mapper.end()) {
        return it->second;
    } else {
        return -1;
    }
}

uint64_t SingleMaterial::GenerateVertexShaderKey(const RenderableItem& item) {
    uint64_t shader_key = 0;
    auto mesh = item.mesh;
    if (!mesh || mesh->position.empty()) {
        LOG_ERROR("A mesh must contain a position stream for rendering");
        return -1;
    }
    if (!mesh->normal0.empty()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_NORMAL;
    }
    if (!mesh->tangent0.empty()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_TANGENT;
    }
    if (!mesh->uv0.empty()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_UV0;
    }
    if (!mesh->uv1.empty()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_UV1;
    }
    if (!mesh->vertex_color0.empty()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_COLOR0;
    }
    if (mesh->IsSkinned() && item.material->IsSkinned()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_SKINNING;
    }
    if (!mesh->morph_targets.empty() && AppGlobalResource::Instance().EnableMorphAnimation()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_MORPHPOSITION;
        if (!mesh->normal0.empty() && !mesh->morph_targets.front()->normal0.empty()) {
            shader_key |= Dielectric::kVSMacros::kVSMacros_MORPHNORMAL;
        }
    }
    if (depth_bias) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_DEPTHBIAS;
    }
    if (AppGlobalResource::Instance().SupportVertexTexture()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_VERTEXTEXTURE;
    }
    if (AppGlobalResource::Instance().SupportFloatTexture()) {
        shader_key |= Dielectric::kVSMacros::kVSMacros_FLOATTEXTURE;
    }
    return shader_key;
}

uint64_t SingleMaterial::GenerateFragmentShaderKey(const RenderableItem& item) { return 0; }

void SingleMaterial::UpdateRenderStates() const { RenderState::Instance() = state; }

void SingleMaterial::SetVertexShaderFile(const std::string& path) {
    if (path.empty()) {
        return;
    }
    auto hint = GetShaderPathHint();
    std::string shader_file = resource::ResourceManager::Instance().GenerateUri(path, hint);
    if (shader_file.empty()) {
        shader_file = resource::ResourceManager::Instance().GenerateUri(
            std::filesystem::path(path).filename().string(), hint);
    }

    if (!shader_file.empty()) {
        vertex_shader_file.swap(shader_file);
    }
}

void SingleMaterial::SetFragmentShaderFile(const std::string& path) {
    if (path.empty()) {
        return;
    }
    auto hint = GetShaderPathHint();
    std::string shader_file = resource::ResourceManager::Instance().GenerateUri(path, hint);
    if (shader_file.empty()) {
        shader_file = resource::ResourceManager::Instance().GenerateUri(
            std::filesystem::path(path).filename().string(), hint);
    }

    if (!shader_file.empty()) {
        fragment_shader_file.swap(shader_file);
    }
}

TextureMap::TextureMap(const TextureMapData& map_data) {
    wrap_mode_u = map_data.wrapModeU;
    wrap_mode_v = map_data.wrapModeV;
    min_filter = map_data.minFilter;
    max_filter = map_data.maxFilter;
    uv_swap = map_data.uvSwap;
    texture = map_data.texture;

    if (map_data.pixelChannels >= 0 && map_data.pixelChannels < 4) {
        pixel_channels[map_data.pixelChannels] = 1.0f;
    } else {
        pixel_channels[0] = 1.0f;
    }

    AssignIfLengthMatch(translation, map_data.T);
    AssignIfLengthMatch(rotation, map_data.R);
    AssignIfLengthMatch(scaling, map_data.S);
    AssignIfLengthMatch(rotation_pivot, map_data.Rp);
    AssignIfLengthMatch(scaling_pivot, map_data.Sp);
}

void TextureMap::LoadImageTexture(const std::vector<std::string>& path_hint, bool do_not_need_pot) {
    std::string file = resource::ResourceManager::Instance().GenerateUri(texture, path_hint);
    if (!texture.empty()) {
        if (!file.empty())
            gl_texture_info = glit::GLTextureInfo::LoadImageTexture(file, do_not_need_pot);
        if (gl_texture_info == nullptr) {
            LOG_ERROR("texture loading maybe failed: %s", texture.c_str());
        }
    }
}

void TextureMap::LoadCubeTexture(const std::vector<std::string>& path_hint, int image_size) {
    std::string file = resource::ResourceManager::Instance().GenerateUri(texture, path_hint);
    if (!texture.empty()) {
        if (!file.empty()) gl_texture_info = glit::GLTextureInfo::LoadCubeTexture(file, image_size);
        if (gl_texture_info == nullptr) {
            LOG_ERROR("texture loading maybe failed: %s", texture.c_str());
        }
    }
}

void TextureMap::LoadPanorama(const std::vector<std::string>& path_hint) {
    std::string file = resource::ResourceManager::Instance().GenerateUri(texture, path_hint);
    if (!texture.empty()) {
        if (!file.empty()) gl_texture_info = glit::GLTextureInfo::LoadPanorama(file);
        if (gl_texture_info == nullptr) {
            LOG_ERROR("texture loading maybe failed: %s", texture.c_str());
        }
    }
}

void TextureMap::LoadIntegratedBRDF(const std::vector<std::string>& path_hint) {
    std::string file = resource::ResourceManager::Instance().GenerateUri(texture, path_hint);
    if (!texture.empty()) {
        if (!file.empty()) gl_texture_info = glit::GLTextureInfo::LoadIntegratedBRDF(file);
        if (gl_texture_info == nullptr) {
            LOG_ERROR("texture loading maybe failed: %s", texture.c_str());
        }
    }
}

bool TextureMap::Valid() const {
    return gl_texture_info != nullptr && gl_texture_info->texture != 0 &&
           gl_texture_info->width != 0 && gl_texture_info->height != 0;
}
