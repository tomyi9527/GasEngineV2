#include "opengl/buffer_type.h"
#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include "FreeImage.h"
#include "data_types/shader_factory.h"
#include "opengl/global_resource.h"
#include "opengl/renderable_item.h"
#include "utils/ipow.h"
#include "utils/json_maker.h"
#include "utils/resource_manager.h"
#include "utils/string_util.h"

// for M_PI
#define _USE_MATH_DEFINES
#include <math.h>

bool ImageResize(const RGBAImageData& src, RGBAImageData& dest) {
    if (dest.width == 0 || dest.height == 0) {
        return false;
    }
    if (dest.width == src.width && dest.height == src.height) {
        dest.data = src.data;
        return true;
    }
    // real rescale
    FIBITMAP* bitmap = FreeImage_Allocate(src.width, src.height, 32);
    auto bytes = (RGBQUAD*)FreeImage_GetBits(bitmap);
    for (uint64_t idx = 0; idx < src.width * src.height; ++idx) {
        bytes[idx].rgbRed = src.data[idx * 4 + 0];       // r
        bytes[idx].rgbGreen = src.data[idx * 4 + 1];     // g
        bytes[idx].rgbBlue = src.data[idx * 4 + 2];      // b
        bytes[idx].rgbReserved = src.data[idx * 4 + 3];  // a
    }
    auto old_bitmap = bitmap;
    bitmap = FreeImage_Rescale(old_bitmap, dest.width, dest.height, FILTER_BILINEAR);
    FreeImage_Unload(old_bitmap);
    dest.data.resize(dest.width * dest.height * 4);
    bytes = (RGBQUAD*)FreeImage_GetBits(bitmap);
    for (uint64_t idx = 0; idx < dest.width * dest.height; ++idx) {
        dest.data[idx * 4 + 0] = bytes[idx].rgbRed;       // r
        dest.data[idx * 4 + 1] = bytes[idx].rgbGreen;     // g
        dest.data[idx * 4 + 2] = bytes[idx].rgbBlue;      // b
        dest.data[idx * 4 + 3] = bytes[idx].rgbReserved;  // a
    }
    FreeImage_Unload(bitmap);
    return true;
}

RGBAImageData GetFrameBufferRGBAImageData(
    const std::shared_ptr<glit::cpp::CustomGLFWContext>& context) {
    RGBAImageData data;
    data.height = context->height;
    data.width = context->width;
    std::string fb = glit::GetFrameBufferRGBAContent(context->width, context->height, context->fb);
    data.data.assign(fb.begin(), fb.end());
    return data;
}

RGBImageData GetFrameBufferRGBImageData(
    const std::shared_ptr<glit::cpp::CustomGLFWContext>& context) {
    RGBImageData data;
    data.height = context->height;
    data.width = context->width;
    std::string fb = glit::GetFrameBufferRGBContent(context->width, context->height, context->fb);
    data.data.assign(fb.begin(), fb.end());
    return data;
}

namespace glit {

GLBufferInfo::~GLBufferInfo() {
    if (last_buffer_size > 0 || opengl_buffer > 0) {
        LOG_DEBUG("Buffer dtor called");
        glDeleteBuffers(1, &opengl_buffer);
    }
}

void GLBufferInfo::ResetGLBufferInfo(kSECTION_TYPE type) {
    opengl_buffer = 0;
    mesh_section_type = type;
    attribute_offset = static_cast<void*>(0);
    attribute_stride = 0;
    is_dynamic = is_dynamic;
    if (type == kPOSITION || type == kNORMAL0 || type == kNORMAL1) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 3;
        component_type = GL_FLOAT;
        component_normalized = GL_FALSE;
        attribute_stride = 3 * sizeof(float);
    } else if (type == kVERTEXCOLOR0 || type == kVERTEXCOLOR1) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 4;
        component_type = GL_UNSIGNED_BYTE;
        component_normalized = GL_TRUE;
        attribute_stride = 4 * sizeof(uint8_t);
    } else if (type == kTANGENT0 || type == kTANGENT1) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 4;
        component_type = GL_FLOAT;
        component_normalized = GL_FALSE;
        attribute_stride = 4 * sizeof(float);
    } else if (type == kUV0 || type == kUV1) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 2;
        component_type = GL_FLOAT;
        component_normalized = GL_FALSE;
        attribute_stride = 2 * sizeof(float);
    } else if (type == kBLENDWEIGHT) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 4;
        component_type = GL_FLOAT;
        component_normalized = GL_FALSE;
        attribute_stride = 4 * sizeof(float);
    } else if (type == kBLENDINDEX) {
        opengl_buffer_type = GL_ARRAY_BUFFER;
        component_count = 4;
        component_type = GL_SHORT;
        component_normalized = GL_FALSE;
        attribute_stride = 4 * sizeof(uint16_t);
    } else if (type == kINDEX) {
        opengl_buffer_type = GL_ELEMENT_ARRAY_BUFFER;
        component_count = 3;
        component_type = GL_UNSIGNED_INT;
        component_normalized = GL_FALSE;
        attribute_stride = 3 * sizeof(uint32_t);
    } else if (type == kTOPOLOGY) {
        opengl_buffer_type = GL_ELEMENT_ARRAY_BUFFER;
        component_count = 2;
        component_type = GL_UNSIGNED_INT;
        component_normalized = GL_FALSE;
        attribute_stride = 2 * sizeof(uint32_t);
    } else if (type == kUVTOPOLOGY) {
        opengl_buffer_type = GL_ELEMENT_ARRAY_BUFFER;
        component_count = 2;
        component_type = GL_UNSIGNED_INT;
        component_normalized = GL_FALSE;
        attribute_stride = 2 * sizeof(uint32_t);
    } else {
        assert(0);
    }
}

void GLBufferInfo::InitBuffer(GLuint VAO_id) {
    if (mesh_target == nullptr) {
        return;
    }
    GLuint draw_mode = is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    std::string_view memory = mesh_target->GetMemoryPtr(mesh_section_type);

    glBindVertexArray(VAO_id);
    // VAO_START
    glGenBuffers(1, &opengl_buffer);
    glBindBuffer(opengl_buffer_type, opengl_buffer);
    // BUFFER_START
    glBufferData(opengl_buffer_type, memory.size(), memory.data(), draw_mode);
    last_buffer_size = memory.size();
    // BUFFER_END
    glBindBuffer(opengl_buffer_type, 0);
    // VAO_END
    glBindVertexArray(0);
}

void GLBufferInfo::InitIndexAttribute(GLuint VAO_id, GLuint attrib_pointer_index) {
    if (last_buffer_size == 0) {
        return;
    }
    glBindVertexArray(VAO_id);
    // VAO_START
    glBindBuffer(opengl_buffer_type, opengl_buffer);
    // INDEX_START
    glVertexAttribPointer(attrib_pointer_index, component_count, component_type,
                          component_normalized, attribute_stride, attribute_offset);
    glEnableVertexAttribArray(attrib_pointer_index);
    // INDEX_END
    glBindBuffer(opengl_buffer_type, 0);
    // VAO_END
    glBindVertexArray(0);
}

void GLBufferInfo::UpdateBuffer(GLuint VAO_id) {
    if (mesh_target == nullptr) {
        return;
    }
    GLuint draw_mode = is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    std::string_view memory = mesh_target->GetMemoryPtr(mesh_section_type);

    glBindVertexArray(VAO_id);
    // VAO_START
    glBindBuffer(opengl_buffer_type, opengl_buffer);
    // BUFFER_START
    glBufferData(opengl_buffer_type, memory.size(), memory.data(), draw_mode);
    last_buffer_size = memory.size();
    // BUFFER_END
    glBindBuffer(opengl_buffer_type, 0);
    // VAO_END
    glBindVertexArray(0);
}

GLVAOInfo::GLVAOInfo() { glGenVertexArrays(1, &vao_id); }

GLVAOInfo::~GLVAOInfo() {
    if (vao_id > 0) {
        LOG_DEBUG("VAO dtor called");
        glDeleteVertexArrays(1, &vao_id);
    }
}

std::shared_ptr<GLTextureInfo> GLTextureInfo::LoadCubeTexture(const std::string& file,
                                                              int image_size) {
    auto it = cache.find(file);
    if (it != cache.end()) {
        return it->second;
    }

    auto hint = GetShaderPathHint();
    std::string content = resource::ResourceManager::Instance().Load(file, hint);

    if (content.empty()) {
        LOG_ERROR("file not found: %s", file.c_str());
        return nullptr;
    }

    std::shared_ptr<GLTextureInfo> ret = std::shared_ptr<GLTextureInfo>(new GLTextureInfo);
    ret->file = file;
    ret->width = image_size;
    ret->height = image_size;
    ret->type = GL_TEXTURE_CUBE_MAP;

    auto images = DecodeCube(content, image_size);
    ret->texture = CreateCubeMapTexture(images);

    cache.emplace(file, ret);

    return ret;
}

std::shared_ptr<GLTextureInfo> GLTextureInfo::LoadPanorama(const std::string& file) {
    auto it = cache.find(file);
    if (it != cache.end()) {
        return it->second;
    }

    auto hint = GetShaderPathHint();
    std::string content = resource::ResourceManager::Instance().Load(file, hint);

    if (content.empty()) {
        LOG_ERROR("file not found: %s", file.c_str());
        return nullptr;
    }

    std::shared_ptr<GLTextureInfo> ret = std::shared_ptr<GLTextureInfo>(new GLTextureInfo);
    ret->file = file;
    ret->type = GL_TEXTURE_2D;

    auto image = DecodePanorama(content);
    ret->width = image.width;
    ret->height = image.height;
    ret->texture = CreatePanoramaTexture(image);

    cache.emplace(file, ret);

    return ret;
}

std::shared_ptr<GLTextureInfo> GLTextureInfo::LoadIntegratedBRDF(const std::string& file) {
    auto it = cache.find(file);
    if (it != cache.end()) {
        return it->second;
    }

    auto hint = GetShaderPathHint();
    std::string content = resource::ResourceManager::Instance().Load(file, hint);

    if (content.empty()) {
        LOG_ERROR("file not found: %s", file.c_str());
        return nullptr;
    }

    std::shared_ptr<GLTextureInfo> ret = std::shared_ptr<GLTextureInfo>(new GLTextureInfo);
    ret->file = file;
    ret->type = GL_TEXTURE_2D;

    auto image = DecodeIntegratedBRDF(content);
    ret->width = image.width;
    ret->height = image.height;
    ret->texture = CreateIntegratedBRDF(image);

    cache.emplace(file, ret);

    return ret;
}

std::shared_ptr<GLTextureInfo> GLTextureInfo::LoadImageTexture(const std::string& file_full_path,
                                                               bool do_not_need_pot) {
    auto it = cache.find(file_full_path);
    if (it != cache.end()) {
        return it->second;
    }

    std::string content = resource::ResourceManager::Instance().Load(file_full_path);
    if (content.empty()) {
        LOG_ERROR("file not found: %s", file_full_path.c_str());
        return nullptr;
    }

    std::shared_ptr<GLTextureInfo> ret = std::shared_ptr<GLTextureInfo>(new GLTextureInfo);
    ret->file = file_full_path;
    ret->type = GL_TEXTURE_2D;

    const std::string ext = std::filesystem::path(file_full_path).extension().string();

    if (StartsWith(file_full_path, "@@") || ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds") {
        // 为了效率在decode时进行了rescale
        auto image = DecodeImage(content, do_not_need_pot);
        ret->width = image.width;
        ret->height = image.height;
        ret->texture = CreateImageTexture(image, do_not_need_pot);
    } else {
        LOG_ERROR("Imgage texture creation failed, unsupported file tye: %s", ext.c_str());
        return nullptr;
    }

    cache.emplace(file_full_path, ret);
    return ret;
}

std::pair<float, std::vector<glm::vec3>> GLTextureInfo::LoadSphericalHarmonics(
    const std::string& file) {
    // 类型不一致，且大小不会很大，不缓存。
    auto hint = GetShaderPathHint();
    std::string content = resource::ResourceManager::Instance().Load(file, hint);
    if (content.empty()) {
        LOG_ERROR("file not found: %s", file.c_str());
        return {0.0f, {}};
    }

    rapidjson::Document doc;
    doc.Parse(content.data(), content.size());
    if (!doc.IsObject()) {
        return {0.0f, {}};
    }
    std::vector<float> diffuse_sph_read;
    json::GetMember(doc, "diffuseSPH", diffuse_sph_read);
    float brightness = 0.0f;
    json::GetMember(doc, "brightness", brightness);

    std::vector<glm::vec3> diffuse_sph = DecodeSphericalHarmonicsCoefficients(diffuse_sph_read);
    if (diffuse_sph.size() == 0) {
        LOG_ERROR("spherical harmonics load error.");
    }

    return std::make_pair(brightness, diffuse_sph);
}

void GLTextureInfo::LoadBoneMatrixTexture(const std::shared_ptr<OBJECT>& obj) {
    if (!AppGlobalResource::Instance().SupportVertexTexture()) {
        // LOG_ERROR("Vertex texture is not supported.");
        return;
    }
    RGBAImageData& global_data = AppGlobalResource::Instance().bone_matrices_storage;
    obj->FillSkinningMatrices(global_data);

    GLuint texture = AppGlobalResource::Instance().bone_texture->texture;
    UpdateBoneTexture(texture, global_data);
}

void GLTextureInfo::UpdateBoneTexture(GLuint texture, const RGBAImageData& data) {
    if (data.width == 0 || data.height == 0 || data.data.empty()) {
        LOG_ERROR("input is empty.");
        return;
    }
    bool support_float_texture = AppGlobalResource::Instance().SupportFloatTexture();
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (support_float_texture) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, data.width, data.height, 0, GL_RGBA, GL_FLOAT,
                     data.data.data());
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data.data.data());
    }

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);

    return;
}

std::shared_ptr<GLTextureInfo> GLTextureInfo::InitEmptyBoneTexture(int size) {
    GLuint texture;
    glGenTextures(1, &texture);
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);

    auto bone_texture = std::shared_ptr<GLTextureInfo>(new GLTextureInfo);
    bone_texture->texture = texture;
    bone_texture->height = size;
    bone_texture->width = size;
    bone_texture->type = GL_TEXTURE_2D;
    bone_texture->file = "global_bone_texture";
    return bone_texture;
}

GLTextureInfo::~GLTextureInfo() {
    if (texture > 0) {
        LOG_DEBUG("texture dtor called");
        glDeleteTextures(1, &texture);
    }
}

std::array<std::vector<RGBAImageData>, 6> GLTextureInfo::DecodeCube(const std::string& content,
                                                                    int image_size) {
    static const double ln2 = std::log(2.0);
    int max_level = std::floor(std::log(image_size) / ln2);
    size_t offset = 0;
    std::array<std::vector<RGBAImageData>, 6> ret;
    for (int i = 0; i <= max_level; ++i) {
        int size = ipow(2, max_level - i);
        if (offset >= content.size()) {
            break;
        }
        for (int face = 0; face < 6; ++face) {
            size_t byte_size = 0;
            // always rgba
            byte_size = size * size * 4;
            const uint8_t* start = reinterpret_cast<const uint8_t*>(content.data() + offset);
            assert(byte_size + offset <= content.size());
            std::vector<uint8_t> image_data(start, start + byte_size);
            std::vector<uint8_t> deinterleave;
            deinterleave.resize(image_data.size());
            int pixel_size = size * size;
            int pixel_size2 = 2 * pixel_size;
            int pixel_size3 = 3 * pixel_size;
            int idx = 0;
            for (int k = 0; k < pixel_size; ++k) {
                deinterleave[idx++] = image_data[k];
                deinterleave[idx++] = image_data[k + pixel_size];
                deinterleave[idx++] = image_data[k + pixel_size2];
                deinterleave[idx++] = image_data[k + pixel_size3];
            }
            ret[face].emplace_back();
            ret[face].back().data = std::move(deinterleave);
            ret[face].back().width = size;
            ret[face].back().height = size;
            offset += byte_size;
        }
    }
    return ret;
}

RGBAImageData GLTextureInfo::DecodePanorama(const std::string& image_data) {
    double size = std::sqrt(image_data.size() / 4);
    RGBAImageData ret;

    std::vector<uint8_t> deinterleave;
    deinterleave.resize(image_data.size());

    int pixel_size = size * size;
    int pixel_size2 = 2 * pixel_size;
    int pixel_size3 = 3 * pixel_size;
    int idx = 0;
    for (int k = 0; k < pixel_size; ++k) {
        deinterleave[idx++] = image_data[k];
        deinterleave[idx++] = image_data[k + pixel_size];
        deinterleave[idx++] = image_data[k + pixel_size2];
        deinterleave[idx++] = image_data[k + pixel_size3];
    }

    ret.data = std::move(deinterleave);
    ret.width = size;
    ret.height = size / 2;

    // 此处的panorama似乎自带mipmap， 但我们先只读取最大的那个。
    // 此处的panorama的y轴是反向的，因此后半部分为最大的图
    ret.data.erase(ret.data.begin(), ret.data.begin() + ret.width * ret.height * 4);
    assert(ret.width * ret.height * 4 == ret.data.size());
    return ret;
}

RGBAImageData GLTextureInfo::DecodeIntegratedBRDF(const std::string& image_data) {
    double size = std::sqrt(image_data.size() / 4);

    RGBAImageData image;
    image.data.assign(image_data.begin(), image_data.end());
    image.width = size;
    image.height = size;

    assert(image.width * image.height * 4 == image.data.size());
    return image;
}

RGBAImageData GLTextureInfo::DecodeImage(const std::string& content, bool do_not_need_pot) {
    RGBAImageData image;

    FIMEMORY* hmem = FreeImage_OpenMemory(
        reinterpret_cast<BYTE*>(const_cast<std::string&>(content).data()), content.size());
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
    FIBITMAP* loaded = FreeImage_LoadFromMemory(fif, hmem, 0);

    if (!loaded) {
        LOG_ERROR("can not load image.");
        FreeImage_CloseMemory(hmem);
        return image;
    }
    image.width = FreeImage_GetWidth(loaded);
    image.height = FreeImage_GetHeight(loaded);
    // // calculate the number of bytes per pixel
    // unsigned bytespp = FreeImage_GetLine(loaded) / FreeImage_GetWidth(loaded);
    // // calculate the number of samples per pixel
    // unsigned channels = bytespp / sizeof(BYTE);  // 每pixel的channel数量

    FIBITMAP* tmp = loaded;
    if (!do_not_need_pot && (!IsPot(image.width) || !IsPot(image.height))) {
        loaded = FreeImage_Rescale(tmp, NearestPot(image.width), NearestPot(image.height),
                                   FILTER_BILINEAR);
        FreeImage_Unload(tmp);
        tmp = loaded;
        image.width = FreeImage_GetWidth(loaded);
        image.height = FreeImage_GetHeight(loaded);
        assert(loaded);
    }

    loaded = FreeImage_ConvertTo32Bits(tmp);
    FreeImage_Unload(tmp);
    tmp = nullptr;

    image.data.resize(image.width * image.height * 4);
    RGBQUAD* pixeles = (RGBQUAD*)FreeImage_GetBits(loaded);
    for (uint64_t j = 0; j < image.width * image.height; j++) {
        image.data[j * 4 + 0] = pixeles[j].rgbRed;
        image.data[j * 4 + 1] = pixeles[j].rgbGreen;
        image.data[j * 4 + 2] = pixeles[j].rgbBlue;
        image.data[j * 4 + 3] = pixeles[j].rgbReserved;
    }

    FreeImage_Unload(loaded);
    FreeImage_CloseMemory(hmem);

    assert(image.width * image.height * 4 == image.data.size());
    return image;
}

std::vector<glm::vec3> GLTextureInfo::DecodeSphericalHarmonicsCoefficients(
    const std::vector<float>& sph_coef) {
    if (sph_coef.size() < 27) {
        return std::vector<glm::vec3>();
    }

    std::vector<float> coef;
    coef.resize(9);
    coef[0] = 1.0 / (2.0 * std::sqrt(M_PI));
    coef[1] = -(std::sqrt(3.0 / M_PI) * 0.5);
    coef[2] = -coef[1];
    coef[3] = coef[1];
    coef[4] = std::sqrt(15.0 / M_PI) * 0.5;
    coef[5] = -coef[4];
    coef[6] = std::sqrt(5.0 / M_PI) * 0.25;
    coef[7] = coef[5];
    coef[8] = std::sqrt(15.0 / M_PI) * 0.25;

    std::vector<glm::vec3> ret;
    ret.resize(9);
    for (int i = 0; i < 9; ++i) {
        ret[i].r = sph_coef[3 * i + 0] * coef[i];
        ret[i].g = sph_coef[3 * i + 1] * coef[i];
        ret[i].b = sph_coef[3 * i + 2] * coef[i];
    }
    return ret;
}

GLuint GLTextureInfo::CreatePanoramaTexture(const RGBAImageData& data) {
    if (data.width == 0 || data.height == 0 || data.data.empty()) {
        LOG_ERROR("input is empty.");
        return 0;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);

    return texture;
}

GLuint GLTextureInfo::CreateIntegratedBRDF(const RGBAImageData& data) {
    if (data.width == 0 || data.height == 0 || data.data.empty()) {
        LOG_ERROR("input is empty.");
        return 0;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);

    return texture;
}

GLuint GLTextureInfo::CreateImageTexture(const RGBAImageData& image, bool do_not_need_pot) {
    if (image.width == 0 || image.height == 0 || image.data.empty()) {
        LOG_ERROR("input is empty.");
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    if (do_not_need_pot || (IsPot(image.width) && IsPot(image.height))) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.data.data());
    } else {
        // resize image to nearest pot
        RGBAImageData resized_image;
        resized_image.width = NearestPot(image.width);
        resized_image.height = NearestPot(image.height);
        if (!ImageResize(image, resized_image)) {
            throw std::runtime_error("image resize after decoding failed.");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resized_image.width, resized_image.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, resized_image.data.data());
    }
    if (do_not_need_pot) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    return texture;
}

GLuint GLTextureInfo::CreateCubeMapTexture(const std::array<std::vector<RGBAImageData>, 6>& data) {
    if (data.empty()) {
        LOG_ERROR("input is empty.");
        return 0;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    // glPixelStorei(GL_UNPACK_FLIP_Y_WEBGL, false);
    GLint active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    GLint texture_binding_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding_2d);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    size_t mipmap_level_count = 0;
    for (uint32_t face = 0; face < 6; ++face) {
        const auto& image = data[face];
        mipmap_level_count = image.size();
        for (size_t level = 0; level < mipmap_level_count; ++level) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGBA, image[level].width,
                         image[level].height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         image[level].data.data());
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    mipmap_level_count > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipmap_level_count - 1);

    glActiveTexture(active_texture);
    glBindTexture(GL_TEXTURE_2D, texture_binding_2d);

    return texture;
}
}  // namespace glit