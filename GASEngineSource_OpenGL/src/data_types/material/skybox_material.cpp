#include "skybox_material.h"
#include "ecs/component/camera_component.h"
#include "opengl/buffer_type.h"
#include "opengl/renderable_item.h"

void SkyBoxMaterial::UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                                    const std::shared_ptr<CameraComponent>& camera,
                                    const RenderableItem& item, const ShaderProgram& sp) const {
    if (camera == nullptr || sp.program == 0 || item.material == nullptr ||
        item.material->GetType() != kSkyboxMaterial) {
        return;
    }
    std::shared_ptr<SkyBoxMaterial> material =
        std::dynamic_pointer_cast<SkyBoxMaterial>(item.material);

    if (sp.fs_key & SkyBox::kFSMacros_CUBEMAP || sp.fs_key & SkyBox::kFSMacros_AMBIENT) {
        glm::vec4 camera_frustum;
        camera_frustum[1] = std::tan(camera->field_of_view * 0.5) * camera->near;  // top
        camera_frustum[0] = camera->aspect * camera_frustum[1];                    // right
        camera_frustum[2] = camera->near;                                          // near
        camera_frustum[3] = camera->far;                                           // far

        glm::mat4 matrix_world = camera->GetWorldMatrix();
        glm::vec3 camera_right = matrix_world[0];
        glm::vec3 camera_up = matrix_world[1];
        glm::vec3 camera_front = matrix_world[2];

        uniform_values["cameraFrustum"] = CopyMemToByteArray(camera_frustum);
        uniform_values["cameraRight"] = CopyMemToByteArray(camera_right);
        uniform_values["cameraUp"] = CopyMemToByteArray(camera_up);
        uniform_values["cameraFront"] = CopyMemToByteArray(camera_front);

        uniform_values["lightExposure"] = CopyMemToByteArray(light_exposure);
        uniform_values["backgroundExposure"] = CopyMemToByteArray(background_exposure);
        uniform_values["orientation"] = CopyMemToByteArray(orientation);
    }

    if (sp.fs_key & SkyBox::kFSMacros_CUBEMAP) {
        uniform_values["cubeMap"] = CopyMemToByteArray(cube_map.gl_texture_info->texture);
        uniform_values["cubeMapSize"] = CopyMemToByteArray(cube_map_size);
    } else if (sp.fs_key & SkyBox::kFSMacros_IMAGE) {
        uniform_values["image"] = CopyMemToByteArray(image.gl_texture_info->texture);
        auto current_context = glit::cpp::CustomGLFWContext::GetCurrentGLFWContext();
        if (current_context != nullptr) {
            float img_aspect =
                (image.gl_texture_info->width / (float)image.gl_texture_info->height);
            float display_aspect = current_context->GetAspect();
            glm::vec2 ratio(display_aspect, img_aspect);
            ratio /= std::max(display_aspect, img_aspect);
            uniform_values["imageRatio"] = CopyMemToByteArray(ratio);
        }
    } else if (sp.fs_key & SkyBox::kFSMacros_AMBIENT) {
        uniform_values["sph"] = CopyMemToByteArray(spherical_harmonics);
    } else {
        uniform_values["solidColor"] = CopyMemToByteArray(solid_color);
    }
}

uint64_t SkyBoxMaterial::GenerateFragmentShaderKey(const RenderableItem& item) {
    uint64_t shader_key = 0;
    auto mesh = item.mesh;
    if (!mesh || mesh->position.empty()) {
        LOG_ERROR("A mesh must contain a position stream for rendering");
        return -1;
    }
    if (background_type == SkyBox::kFSMacros_CUBEMAP && cube_map.Valid()) {
        shader_key |= SkyBox::kFSMacros_CUBEMAP;
    } else if (background_type == SkyBox::kFSMacros_IMAGE && image.Valid()) {
        shader_key |= SkyBox::kFSMacros_IMAGE;
    } else if (background_type == SkyBox::kFSMacros_AMBIENT && spherical_harmonics.size() == 9) {
        shader_key |= SkyBox::kFSMacros_AMBIENT;
    } else {
        shader_key |= SkyBox::kFSMacros_SOLIDCOLOR;
    }
    return shader_key;
}

void SkyBoxMaterial::ApplyPresetSolidColor() {
    background_type = SkyBox::kFSMacros_SOLIDCOLOR;
    SetSolidColor(0.4, 0.4, 0.5);
}
void SkyBoxMaterial::ApplyPresetImage() {
    background_type = SkyBox::kFSMacros_IMAGE;
    image.texture = "backgroundImages/SYSTEM_DARK_2.jpg";
    // image.texture = "backgroundImages/image-2048x1024.jpg";
    image.LoadImageTexture(GetShaderPathHint(), true);
}
void SkyBoxMaterial::ApplyPresetCubeMap() {
    background_type = SkyBox::kFSMacros_CUBEMAP;
    cube_map.texture =
        "backgroundCubes/01_attic_room_with_windows/background_cubemap_512_0.02_luv.bin";
    cube_map.LoadCubeTexture(GetShaderPathHint(), 512);
    cube_map_size[0] = cube_map.gl_texture_info->width;
}
void SkyBoxMaterial::ApplyPresetAmbient() {
    background_type = SkyBox::kFSMacros_AMBIENT;
    auto sph_loaded = glit::GLTextureInfo::LoadSphericalHarmonics(
        "backgroundCubes/01_attic_room_with_windows/diffuse_sph.json");
    SetSphericalHarmonics(sph_loaded.first, sph_loaded.second);
}