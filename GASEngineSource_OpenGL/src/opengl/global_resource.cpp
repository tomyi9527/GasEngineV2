#include "opengl/global_resource.h"
#include <math.h>
#include <algorithm>
#include "data_types/geometry.h"
#include "opengl/buffer_type.h"
#include "opengl/opengl_interface.h"
#include "data_types/shader_factory.h"

void AppGlobalResource::InitializeUnitMesh() { 
    SphereMesh::UnitMesh()->SubmitToOpenGL();
    PyramidMesh::UnitMesh()->SubmitToOpenGL();
}

void AppGlobalResource::DeinitializeUnitMesh() { 
    SphereMesh::UnitMesh()->vao_info.reset(); 
    PyramidMesh::UnitMesh()->vao_info.reset();
}

void AppGlobalResource::UpdateFlags() {
    // 1
    int max_vertex_texture;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_vertex_texture);
    support_vertex_texture = max_vertex_texture > 0;
    // 2
    support_float_texture = glit::HasExt("GL_ARB_texture_float");
    // 3
    support_texture_lod = glit::HasExt("GL_ARB_shader_texture_lod");
}

void AppGlobalResource::CreateEmptyBoneTexture() {
    if (support_vertex_texture) {
        if (support_float_texture) {
            // layout (1 matrix = 4 pixels)
            //      RGBA RGBA RGBA RGBA (=> column1, column2, column3, column4)
            //  with  8x8  pixel texture max   16 bones * 4 pixels =  (8 * 8)
            //       16x16 pixel texture max   64 bones * 4 pixels = (16 * 16)
            //       32x32 pixel texture max  256 bones * 4 pixels = (32 * 32)
            //       64x64 pixel texture max 1024 bones * 4 pixels = (64 * 64)

            int32_t size =
                std::ceil(std::sqrt(max_bone_count * 4));  // 4 pixels needed for 1 matrix
            size = std::max(glit::GLTextureInfo::NextPot(size), 4);
            bone_texture = glit::GLTextureInfo::InitEmptyBoneTexture(size);
            if (bone_texture) {
                bone_matrices_storage.height = size;
                bone_matrices_storage.width = size;
                bone_matrices_storage.data.resize(size * size * 4 * sizeof(float),
                                                  0);  // 4 floats per RGBA pixel
            }
        } else {
            // layout (1 matrix = 32 pixels)
            int32_t size =
                std::ceil(std::sqrt(max_bone_count * 32));  // 32 pixels needed for 1 matrix
            size = std::max(glit::GLTextureInfo::NextPot(size), 32);
            bone_texture = glit::GLTextureInfo::InitEmptyBoneTexture(size);
            if (bone_texture) {
                bone_matrices_storage.height = size;
                bone_matrices_storage.width = size;
                bone_matrices_storage.data.resize(size * size * 4 * sizeof(uint8_t),
                                                  0);  // 4 bytes per RGBA pixel
            }
        }
    }
}

void AppGlobalResource::Terminate() {
    bone_texture.reset();
    ShaderFactory::Instance().DestroyPrograms();
    glit::GLTextureInfo::DestroyCache();
    DeinitializeUnitMesh();
    initialized = false;
}