#pragma once
#include <map>
#include <memory>
#include <vector>
#include "data_types/material_loader.h"
#include "data_types/mesh_loader.h"
#include "data_types/shader_factory.h"
#include "glm/glm.hpp"
#include "opengl/opengl_interface.h"
#include "utils/image_data.h"

bool ImageResize(const RGBAImageData& src, RGBAImageData& dest);
RGBAImageData GetFrameBufferRGBAImageData(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context);
RGBImageData GetFrameBufferRGBImageData(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context);

namespace glit {

class GLTextureInfo {
 public:
    static std::shared_ptr<GLTextureInfo> LoadCubeTexture(const std::string& file, int image_size);
    static std::shared_ptr<GLTextureInfo> LoadPanorama(const std::string& file);
    static std::shared_ptr<GLTextureInfo> LoadIntegratedBRDF(const std::string& file);
    static std::shared_ptr<GLTextureInfo> LoadImageTexture(const std::string& file,
                                                           bool do_not_need_pot = false);
    static std::pair<float, std::vector<glm::vec3>> LoadSphericalHarmonics(const std::string& file);
    // load from memory
    static std::shared_ptr<GLTextureInfo> InitEmptyBoneTexture(int size);
    static void LoadBoneMatrixTexture(const std::shared_ptr<OBJECT>& obj);
    static void UpdateBoneTexture(GLuint texture, const RGBAImageData& data);
    static void DestroyCache() {
        cache.clear();
    }

    ~GLTextureInfo();

    std::string file;
    GLuint texture = 0;
    int width = 0;
    int height = 0;
    GLuint type = 0;  // texture_type, GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP

 protected:
    GLTextureInfo() {}
    // from file
    static std::array<std::vector<RGBAImageData>, 6> DecodeCube(const std::string& content,
                                                                int image_size);
    static RGBAImageData DecodePanorama(const std::string& content);
    static RGBAImageData DecodeIntegratedBRDF(const std::string& content);
    static RGBAImageData DecodeImage(const std::string& content, bool do_not_need_pot = false);
    // need total 27 floats (3 * 9)
    static std::vector<glm::vec3> DecodeSphericalHarmonicsCoefficients(
        const std::vector<float>& sph_coef);

    // create cube texture in opengl
    static GLuint CreateCubeMapTexture(const std::array<std::vector<RGBAImageData>, 6>& data);
    static GLuint CreatePanoramaTexture(const RGBAImageData& data);
    static GLuint CreateIntegratedBRDF(const RGBAImageData& data);
    static GLuint CreateImageTexture(const RGBAImageData& image, bool do_not_need_pot);

    static std::map<std::string, std::shared_ptr<GLTextureInfo>> cache;

 public:
    static bool IsPot(int val) { return (val & (val - 1)) == 0 && val != 0; }
    static int NearestPot(int val) {
        static double ln2 = std::log(2);
        int expo = std::round(std::log(val) / ln2);
        return std::pow(2, expo);
    }
    static int32_t NextPot(int32_t val) {
        val--;               // ...00001XXXXXXXXXXXXXXXXXXXXXXXXXXX...
        val |= (val >> 1);   // ...000011XXXXXXXXXXXXXXXXXXXXXXXXXX...
        val |= (val >> 2);   // ...00001111XXXXXXXXXXXXXXXXXXXXXXXX...
        val |= (val >> 4);   // ...000011111111XXXXXXXXXXXXXXXXXXXX...
        val |= (val >> 8);   // ...00001111111111111111XXXXXXXXXXXX...
        val |= (val >> 16);  // ...00001111111111111111111111111111...
        val++;               // ...00010000000000000000000000000000...
        return val;
    }
};

class GLBufferInfo {
 public:
    GLBufferInfo(kSECTION_TYPE type) noexcept { ResetGLBufferInfo(type); }
    ~GLBufferInfo();

    GLBufferInfo(const GLBufferInfo& rhs) = delete;
    GLBufferInfo& operator=(const GLBufferInfo& rhs) = delete;
    GLBufferInfo(GLBufferInfo&& rhs) noexcept {
        // user input these
        mesh_section_type = rhs.mesh_section_type;
        is_dynamic = rhs.is_dynamic;
        mesh_target = std::move(rhs.mesh_target);

        // program generates these
        opengl_buffer_type = rhs.opengl_buffer_type;
        component_count = rhs.component_count;
        component_type = rhs.component_type;
        component_normalized = rhs.component_normalized;
        attribute_stride = rhs.attribute_stride;
        attribute_offset = rhs.attribute_offset;

        opengl_buffer = rhs.opengl_buffer;
        last_buffer_size = rhs.last_buffer_size;
        rhs.opengl_buffer = 0;  // need clear
        rhs.last_buffer_size = 0;
    }
    GLBufferInfo& operator=(GLBufferInfo&& rhs) noexcept {
        // user input these
        mesh_section_type = rhs.mesh_section_type;
        is_dynamic = rhs.is_dynamic;
        mesh_target = std::move(rhs.mesh_target);

        // program generates these
        opengl_buffer_type = rhs.opengl_buffer_type;
        component_count = rhs.component_count;
        component_type = rhs.component_type;
        component_normalized = rhs.component_normalized;
        attribute_stride = rhs.attribute_stride;
        attribute_offset = rhs.attribute_offset;

        opengl_buffer = rhs.opengl_buffer;
        last_buffer_size = rhs.last_buffer_size;
        rhs.opengl_buffer = 0;  // need clear
        rhs.last_buffer_size = 0;

        return *this;
    }

    void ResetGLBufferInfo(kSECTION_TYPE type);
    void SetDynamic(bool dynamic) { is_dynamic = dynamic; }
    void SetMeshTarget(const std::shared_ptr<OBJECT>& ptr) { mesh_target = ptr; }
    void InitBuffer(GLuint VAO_id);
    void InitIndexAttribute(GLuint VAO_id, GLuint attrib_pointer_index);
    void UpdateBuffer(GLuint VAO_id);

    GLuint GetID() const { return opengl_buffer; }

    // user input these
    kSECTION_TYPE mesh_section_type;
    bool is_dynamic;
    std::shared_ptr<OBJECT> mesh_target;

    // program generates these
    GLuint opengl_buffer_type;
    uint32_t component_count;
    GLuint component_type;
    GLboolean component_normalized;
    uint32_t attribute_stride;
    void* attribute_offset;

 protected:
    GLuint opengl_buffer;
    uint32_t last_buffer_size = 0;
};

class GLVAOInfo {
 public:
    GLVAOInfo();
    ~GLVAOInfo();

    GLVAOInfo(const GLVAOInfo& rhs) = delete;
    GLVAOInfo& operator=(const GLVAOInfo& rhs) = delete;
    GLVAOInfo(GLVAOInfo&& rhs) noexcept {
        relative_buffers = std::move(rhs.relative_buffers);
        vao_id = rhs.vao_id;
        rhs.vao_id = 0;
    }
    GLVAOInfo& operator=(GLVAOInfo&& rhs) noexcept {
        relative_buffers = std::move(rhs.relative_buffers);
        vao_id = rhs.vao_id;
        rhs.vao_id = 0;
        return *this;
    }

    GLuint GetID() const { return vao_id; }
    const std::map<kSECTION_TYPE, GLBufferInfo>& GetBuffers() const { return relative_buffers; }
    std::map<kSECTION_TYPE, GLBufferInfo>& GetBuffers() { return relative_buffers; }

 protected:
    GLuint vao_id = 0;
    std::map<kSECTION_TYPE, GLBufferInfo> relative_buffers;
};
}  // namespace glit