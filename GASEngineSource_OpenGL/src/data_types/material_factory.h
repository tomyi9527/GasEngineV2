#pragma once

#include <assert.h>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "glm/gtc/type_ptr.hpp"
#include "opengl/opengl_interface.h"
#include "utils/logger.h"
#include "utils/resource_manager.h"

class RenderableItem;
class ShaderProgram;
class CameraComponent;
class MeshFilterComponent;
class TextureMapData;
namespace glit {
class GLTextureInfo;
}

class UniformValueStorage {
 public:
    std::string mem;
    int element_size = 0;
    int count = 0;
};

// helper function
namespace glm {
template <typename T, qualifier Q>
T const* value_ptr(vec<1, T, Q> const& v) {
    return &(v.x);
}
}  // namespace glm

template <typename GLM_TYPE>
inline UniformValueStorage CopyMemToByteArray(const GLM_TYPE& val, int element_size = 0) {
    UniformValueStorage ret;
    ret.mem = std::string(reinterpret_cast<const char*>(glm::value_ptr(val)), sizeof(GLM_TYPE));
    if (element_size == 0)
        ret.element_size = sizeof(GLM_TYPE);
    else
        ret.element_size = element_size;
    ret.count = ret.mem.size() / ret.element_size;
    return ret;
}

template <typename ValueType>
inline UniformValueStorage CopyMemToByteArray(const std::vector<ValueType>& val,
                                              int element_size = 0) {
    UniformValueStorage ret;
    ret.mem =
        std::string(reinterpret_cast<const char*>(val.data()), val.size() * sizeof(ValueType));
    if (element_size == 0)
        ret.element_size = sizeof(ValueType);
    else
        ret.element_size = element_size;
    ret.count = ret.mem.size() / ret.element_size;
    return ret;
}

inline UniformValueStorage CopyMemToByteArray(const GLuint& val) {
    UniformValueStorage ret;
    ret.mem = std::string(reinterpret_cast<const char*>(&val), sizeof(GLuint));
    ret.element_size = sizeof(GLuint);
    ret.count = 1;
    return ret;
}

inline UniformValueStorage CopyMemToByteArray(const GLint& val) {
    UniformValueStorage ret;
    ret.mem = std::string(reinterpret_cast<const char*>(&val), sizeof(GLint));
    ret.element_size = sizeof(GLint);
    ret.count = 1;
    return ret;
}

inline UniformValueStorage CopyMemToByteArray(const float& val) {
    UniformValueStorage ret;
    ret.mem = std::string(reinterpret_cast<const char*>(&val), sizeof(float));
    ret.element_size = sizeof(float);
    ret.count = 1;
    return ret;
}

template <int Dims, typename ValueTypeDst, typename ValueTypeSrc>
void AssignIfLengthMatch(glm::vec<Dims, ValueTypeDst, glm::defaultp>& dst,
                         const std::vector<ValueTypeSrc>& src) {
    if (src.size() != Dims) {
        LOG_WARNING("data provided length is %lu, but memory layout length in class is %d",
                    src.size(), Dims);
    }
    for (size_t i = 0; i < std::min(src.size(), static_cast<size_t>(Dims)); ++i) {
        dst[i] = src[i];
    }
}

enum kMaterialType {
    kBlinnPhongMaterial,
    kDielectricMaterial,
    kElectricMaterial,
    kMatCapMaterial,
    kCompoundMaterial,
    kSkyboxMaterial,
    kWireframeMaterial,
    kPureColorMaterial,
    kDepthMaterial,
    kHotspotMaterial,
    kUVLayoutMaterial,
    kLambertianMaterial,
    kXYPureColorMaterial,

    kMaterialTypeCount
};

constexpr const char* path_hint[] = {"",
                                     ".",
                                     "resources",
                                     "resources/shaders",
                                     "../resources",
                                     "../resources/shaders",
                                     "../../resources",
                                     "../../resources/shaders"};

inline std::vector<std::string> GetShaderPathHint() {
    return std::vector<std::string>(path_hint,
                                    path_hint + (sizeof(path_hint) / sizeof(path_hint[0])));
}

class Material {
 public:
    Material(kMaterialType in_type) : type(in_type) {}
    virtual ~Material() {}

    std::string GetName() const { return name; }
    void SetName(const std::string& in_name) { name = in_name; }

    kMaterialType GetType() const { return type; }
    std::string GetTypeStr() const;

    virtual bool IsTransparencyEnabled() const { return false; }

    bool IsVisible() const { return is_visible; }
    void SetVisible(bool in) { is_visible = in; }

    bool IsWireFrame() const { return is_wireframe; }
    void SetWireFrame(bool in) { is_wireframe = in; }
    bool IsShowUVLayout() const { return is_show_uvlayout; }
    void SetShowUVLayout(bool in) { is_show_uvlayout = in; }
    bool IsSkinned() const { return is_skinned; }
    void SetSkinned(bool in) { is_skinned = in; }

    virtual void SetDepthTest(bool enabled) = 0;

    virtual uint64_t GenerateVertexShaderKey(const RenderableItem& item) = 0;
    virtual uint64_t GenerateFragmentShaderKey(const RenderableItem& item) = 0;

    virtual const std::string& GetVertexShaderFile() const = 0;
    virtual const std::string& GetFragmentShaderFile() const = 0;
    virtual const std::string& GetVertexShaderContent() const = 0;
    virtual const std::string& GetFragmentShaderContent() const = 0;

    virtual void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                const std::shared_ptr<CameraComponent>& camera,
                                const RenderableItem& item, const ShaderProgram& sp) const = 0;

    virtual void LoadAllContent() = 0; 
    virtual void UpdateRenderStates() const = 0;

 protected:
    std::string name;
    kMaterialType type;
    bool is_transparency_enabled = false;
    bool is_visible = true;
    bool is_wireframe = false;
    bool is_show_uvlayout = false;
    bool is_skinned = true;
};

class SingleMaterial : public Material {
 public:
    SingleMaterial(kMaterialType in_type) : Material(in_type) {}

    virtual uint64_t GenerateVertexShaderKey(const RenderableItem& item) override;
    virtual uint64_t GenerateFragmentShaderKey(const RenderableItem& item) override;

    virtual const std::string& GetVertexShaderFile() const override { return vertex_shader_file; }
    virtual const std::string& GetFragmentShaderFile() const override {
        return fragment_shader_file;
    }
    virtual const std::string& GetVertexShaderContent() const override {
        if (vertex_shader_content.empty() && !try_loaded) {
            LOG_ERROR("please load shader content before get it.");
        }
        return vertex_shader_content;
    }
    virtual const std::string& GetFragmentShaderContent() const override {
        if (fragment_shader_content.empty() && !try_loaded) {
            LOG_ERROR("please load shader content before get it.");
        }
        return fragment_shader_content;
    }
    virtual void UpdateRenderStates() const override;

    void SetVertexShaderFile(const std::string& path);
    void SetFragmentShaderFile(const std::string& path);

    virtual void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                const std::shared_ptr<CameraComponent>& camera,
                                const RenderableItem& item, const ShaderProgram& sp) const = 0;

    virtual void LoadAllContent() override {
        try_loaded = true;
        if (!vertex_shader_file.empty()) {
            vertex_shader_content = LoadContentByFile(vertex_shader_file);
        }
        assert(!vertex_shader_content.empty());

        if (!fragment_shader_file.empty()) {
            fragment_shader_content = LoadContentByFile(fragment_shader_file);
        }
        assert(!fragment_shader_content.empty());
    }

    virtual void SetDepthTest(bool enabled) override { state.flag_depth_test = enabled; }

 protected:
    static std::string LoadContentByFile(const std::string& file) {
        std::string& cache_content = cache[file];
        if (cache_content.empty()) {
            cache_content = resource::ResourceManager::Instance().Load(file);
            if (!cache_content.empty()) {
                LOG_INFO("material load content from: %s", file.c_str());
            }
        }
        return cache_content;
    }

    mutable RenderState state;

    std::string name;
    int64_t unique_id = -1;

    std::string vertex_shader_file;
    std::string fragment_shader_file;

    std::string vertex_shader_content;
    std::string fragment_shader_content;

    bool depth_bias = false;
    bool vertex_color = true;

    bool is_mutable = false;
    bool try_loaded = false;

    bool highlight_mask_enable = false;
    glm::vec4 highlight_mask_color = glm::vec4(0.8, 0.8, 0.5, 0.0);

    static std::map<std::string, std::string> cache;
};

class TextureMap {
 public:
    TextureMap() {}
    TextureMap(const TextureMapData& map_data);

    void LoadImageTexture(const std::vector<std::string>& path_hint, bool do_not_need_pot = false);
    void LoadCubeTexture(const std::vector<std::string>& path_hint, int image_size);
    void LoadPanorama(const std::vector<std::string>& path_hint);
    void LoadIntegratedBRDF(const std::vector<std::string>& path_hint);
    bool Valid() const;

 public:
    std::string texture;
    glm::vec4 pixel_channels = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    std::string wrap_mode_u;
    std::string wrap_mode_v;
    std::string min_filter;
    std::string max_filter;
    bool uv_swap = false;
    glm::vec3 translation = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scaling = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 rotation_pivot = glm::vec3(0.0f);
    glm::vec3 scaling_pivot = glm::vec3(0.0f);
    std::shared_ptr<glit::GLTextureInfo> gl_texture_info;
};

class MaterialFactory {
 public:
    static MaterialFactory& Instance() {
        static MaterialFactory instance;
        return instance;
    }
    // std::shared_ptr<Component> Create(const std::string& name, uint64_t uid);
    std::shared_ptr<Material> Create(kMaterialType type);
    // delete record only
    // void Destroy(const std::shared_ptr<Material>& ptr) {
    //     auto it = used_materials.find(ptr);
    //     if (it != used_materials.end()) {
    //         used_materials.erase(it);
    //     }
    // }
    static int TypeFromString(std::string type);

 private:
    MaterialFactory() {}
    // std::set<std::shared_ptr<Material>> used_materials;
};

using pMaterial = std::shared_ptr<Material>;