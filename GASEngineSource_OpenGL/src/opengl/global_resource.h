#pragma once
#include "opengl/buffer_type.h"

class AppGlobalResource {
 public:
    constexpr static const int max_bone_count = 1024;
    static AppGlobalResource& Instance() {
        static AppGlobalResource instance;
        if (!instance.Valid()) {
            instance.Init();
        }
        return instance;
    }

    bool Valid() const { return initialized; }

    bool SupportVertexTexture() const { return support_vertex_texture; }
    bool SupportFloatTexture() const { return support_float_texture; }
    bool SupportTextureLod() const { return support_texture_lod; }

    bool EnableSkinning() const { return enable_skinning; }
    bool EnableMorphAnimation() const { return enable_morph_animation; }
    bool EnableMotionBuilderParameters() const { return enable_motion_builder_parameters; }

    void SetEnableSkinning(bool in) {
        // skinning requires mb_props
        enable_skinning = in;
        enable_motion_builder_parameters = in;
    }
    void SetEnableMorphAnimation(bool in) { enable_morph_animation = in; }
    void SetEnableMotionBuilderParameters(bool in) { enable_motion_builder_parameters = in; }
    
    // call this after init glit and create current context
    void Init() {
        if (initialized) {
            return;
        }
        InitializeUnitMesh();
        UpdateFlags();
        CreateEmptyBoneTexture();
        initialized = true;
    }
    // call this before terminate glit
    void Terminate();

 public:
    std::shared_ptr<glit::GLTextureInfo> bone_texture;
    RGBAImageData bone_matrices_storage;

 protected:
    void InitializeUnitMesh();
    void DeinitializeUnitMesh();
    void UpdateFlags();
    void CreateEmptyBoneTexture();

    bool initialized = false;
    bool support_vertex_texture = false;
    bool support_float_texture = false;
    bool support_texture_lod = false;

    bool enable_skinning = true;
    bool enable_morph_animation = true;
    bool enable_motion_builder_parameters = true;
};