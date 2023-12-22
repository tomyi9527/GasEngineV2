#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include "data_types/material/blinn_phong_material.h"
#include "data_types/material/pure_color_material.h"
#include "data_types/material/xy_only_pure_color_material.h"
#include "data_types/material_factory.h"
#include "data_types/mesh_loader.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component_factory.h"
#include "opengl/buffer_type.h"
#include "opengl/renderable_item.h"
#include "utils/bmp.h"
#include "utils/logger.h"

constexpr char target_mesh_file[] =
    "../../resources/model/girlwalk/gas2/girlwalk.fbx.model_body.326.mesh.bin";

#define PURECOLOR_NOPROJECTION
#define PURECOLOR_PROJECTION

constexpr int total_time = 2;            // 2s
constexpr int frames = total_time * 60;  // per second

GLFWwindow* window = nullptr;

int test_purecolor_nocamera_simple_triangles();
int test_purecolor_nocamera_loadmodel();
int test_purecolor_camera_simple_triangles();
int test_purecolor_camera_loadmodel();
int test_pbr_camera_loadmodel();
int test_uvlayout_camera_loadmodel();

#ifdef USE_OSMESA
bool osmesa = true;
#else
bool osmesa = false;
#endif

#ifdef WIN32
#define _put_env(p, s) _putenv_s(p, s)
#define _get_env(p) getenv(p)
#else
#define _put_env(p, s)        \
    do {                      \
        if (p) {              \
            std::string m(p); \
            putenv(m.data()); \
        }                     \
    } while (0)
#define _get_env(p) getenv(p)
#endif

inline void set_env(const std::string& key, const std::string& value) {
    _put_env(key.c_str(), value.c_str());
}

inline std::string get_env(const std::string& key) {
    char* ptr = _get_env(key.c_str());
    if (ptr == nullptr)
        return std::string();
    else
        return std::string(ptr);
}

inline void SaveScreen(GLuint fb, int width, int height, const std::string& filename) {
    std::string buffer = glit::GetFrameBufferRGBContent(width, height, fb);
    BMP bmp(width, height);
    bmp.FromRGB(width, height, buffer.data());
    bmp.save(filename);
}

int main() {
    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;

    if (osmesa) {
        if (get_env("GALLIUM_DRIVER").empty() || get_env("MESA_GL_VERSION_OVERRIDE").empty() ||
            get_env("MESA_GLSL_VERSION_OVERRIDE").empty()) {
            LOG_ERROR("you can try setting following env for higher opengl version:");
            LOG_ERROR("LIBGL_ALWAYS_SOFTWARE=true    ");
            LOG_ERROR("GALLIUM_DRIVER=llvmpipe       ");
            LOG_ERROR("MESA_GL_VERSION_OVERRIDE=4.6  ");
            LOG_ERROR("MESA_GLSL_VERSION_OVERRIDE=460");
            LOG_ERROR("MESA_EXTENSION_OVERRIDE=GL_ARB_shader_texture_lod");
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    // 0. init
    glit::Init();
    {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->width = 1000;
        context->height = 800;
        context->CreateContext(osmesa);
        glit::PrintExtensions();

        GLint fb = context->fb;
        window = context->window;

#ifdef PURECOLOR_NOPROJECTION
        test_purecolor_nocamera_simple_triangles();
        SaveScreen(fb, context->width, context->height,
                   "test_purecolor_nocamera_simple_triangles.bmp");
        test_purecolor_nocamera_loadmodel();
        SaveScreen(fb, context->width, context->height, "test_purecolor_nocamera_loadmodel.bmp");
#endif
#ifdef PURECOLOR_PROJECTION
        test_purecolor_camera_simple_triangles();
        SaveScreen(fb, context->width, context->height,
                   "test_purecolor_camera_simple_triangles.bmp");
        test_purecolor_camera_loadmodel();
        SaveScreen(fb, context->width, context->height, "test_purecolor_camera_loadmodel.bmp");
#endif
        test_pbr_camera_loadmodel();
        SaveScreen(fb, context->width, context->height, "test_pbr_camera_loadmodel.bmp");
        test_uvlayout_camera_loadmodel();
        SaveScreen(fb, context->width, context->height, "test_uvlayout_camera_loadmodel.bmp");
    }
    // final. clean
    glit::Terminate();

    return 0;
}

inline void PrintMat4(const glm::mat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) std::cout << mat[j][i] << "\t";
        std::cout << std::endl;
    }
}

// show 2 simple triangles pure color  without camera
int test_purecolor_nocamera_simple_triangles() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);
    auto pobj = OBJECT::GenerateOBJECT();
    pobj->position = {
        -0.90f, -0.90f, 0.0f, 0.85f, -0.90f, 0.0f, -0.90f, 0.85f, 0.0f,
        0.90f,  -0.85f, 0.0f, 0.90f, 0.90f,  0.0f, -0.85f, 0.90f, 0.0f,
    };
    pobj->section_item_map[kPOSITION] =
        SECTION_ITEM();  // note: item content doesn't matter currently
    pobj->indices = {0, 1, 2, 3, 4, 5};
    pobj->section_item_map[kINDEX] = SECTION_ITEM();
    pobj->normal0 = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    pobj->section_item_map[kNORMAL0] = SECTION_ITEM();
    pobj->submesh.push_back(SUBMESH(0, 2));
    pobj->section_item_map[kSUBMESH] = SECTION_ITEM();
    pobj->SubmitToOpenGL();
    mesh_filter_component->SetMesh(pobj);
    mesh_filter_component->UpdateBBox();

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kXYPureColorMaterial);
    auto pure_color_mat = std::dynamic_pointer_cast<XYPureColorMaterial>(mat);
    pure_color_mat->LoadAllContent();

    // 3. default camera, won't affect result
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);

    // 4. renderable
    camera_component->AppendRenderables(mesh_filter_component->GetMesh(), pure_color_mat,
                                        mesh_filter_component->GetMesh()->submesh.front(), nullptr,
                                        std::set<std::shared_ptr<PunctualLightComponent>>(),
                                        std::set<std::shared_ptr<DirectionalLightComponent>>(),
                                        std::set<std::shared_ptr<PointLightComponent>>(),
                                        std::set<std::shared_ptr<SpotLightComponent>>(),
                                        glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = pure_color_mat->GenerateVertexShaderKey(item);
        auto fskey = pure_color_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}

// show model pure color  without camera
int test_purecolor_nocamera_loadmodel() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);

    std::fstream file_stream(target_mesh_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_mesh_file);
        return -1;
    }
    mesh_filter_component->LoadMesh(file_stream);
    float max = 1.0 * glm::compMax(2.0f * mesh_filter_component->GetBBox().GetRadius());
    for (auto& m : mesh_filter_component->GetMesh()->position) {
        m /= max;
    }
    mesh_filter_component->GetMesh()->SubmitToOpenGL();

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kXYPureColorMaterial);
    auto pure_color_mat = std::dynamic_pointer_cast<XYPureColorMaterial>(mat);
    pure_color_mat->LoadAllContent();

    // 3. default camera, won't affect result
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);

    // 4. renderable
    camera_component->AppendRenderables(mesh_filter_component->GetMesh(), pure_color_mat,
                                        mesh_filter_component->GetMesh()->submesh.front(), nullptr,
                                        std::set<std::shared_ptr<PunctualLightComponent>>(),
                                        std::set<std::shared_ptr<DirectionalLightComponent>>(),
                                        std::set<std::shared_ptr<PointLightComponent>>(),
                                        std::set<std::shared_ptr<SpotLightComponent>>(),
                                        glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = pure_color_mat->GenerateVertexShaderKey(item);
        auto fskey = pure_color_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }
        std::cout << ".";

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}

// show 2 simple triangles pure color  with camera
int test_purecolor_camera_simple_triangles() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);
    auto pobj = OBJECT::GenerateOBJECT();
    pobj->position = {
        -0.90f, -0.90f, 0.0f, 0.85f, -0.90f, 0.0f, -0.90f, 0.85f, 0.0f,
        0.90f,  -0.85f, 0.0f, 0.90f, 0.90f,  0.0f, -0.85f, 0.90f, 0.0f,
    };
    pobj->section_item_map[kPOSITION] =
        SECTION_ITEM();  // note: item content doesn't matter currently
    pobj->indices = {0, 1, 2, 3, 4, 5};
    pobj->section_item_map[kINDEX] = SECTION_ITEM();
    pobj->normal0 = {
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    pobj->section_item_map[kNORMAL0] = SECTION_ITEM();
    pobj->submesh.push_back(SUBMESH(0, 2));
    pobj->section_item_map[kSUBMESH] = SECTION_ITEM();
    pobj->SubmitToOpenGL();
    mesh_filter_component->SetMesh(pobj);
    mesh_filter_component->UpdateBBox();

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kPureColorMaterial);
    auto pure_color_mat = std::dynamic_pointer_cast<PureColorMaterial>(mat);
    pure_color_mat->LoadAllContent();

    // 3. default camera
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);
    camera_component->SetViewMatrix(glm::mat4(1.0f));
    camera_component->SetProjectionMatrix(glm::mat4(1.0f));
    camera_component->SetProjectionMatrix(
        kCameraType::kPerspective, glm::radians(45.0f),
        glit::cpp::CustomGLFWContext::GetCurrentGLFWContext()->GetAspect(), 0.2, 2000.0);
    glm::vec3 mesh_center = mesh_filter_component->GetBBox().GetCenter();
    glm::vec3 camera_position = mesh_filter_component->GetBBox().GetCenter() +
                                mesh_filter_component->GetBBox().GetRadius() * 2.5f;
    camera_component->SetViewMatrix(camera_position, mesh_center);

    // 4. renderable
    camera_component->AppendRenderables(mesh_filter_component->GetMesh(), pure_color_mat,
                                        mesh_filter_component->GetMesh()->submesh.front(), nullptr,
                                        std::set<std::shared_ptr<PunctualLightComponent>>(),
                                        std::set<std::shared_ptr<DirectionalLightComponent>>(),
                                        std::set<std::shared_ptr<PointLightComponent>>(),
                                        std::set<std::shared_ptr<SpotLightComponent>>(),
                                        glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = pure_color_mat->GenerateVertexShaderKey(item);
        auto fskey = pure_color_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camera_position(0.0f, 0.0f, 2.5f);
        camera_position.x = mesh_filter_component->GetBBox().GetCenter().x +
                            mesh_filter_component->GetBBox().GetRadius().x * 2 + 0.001f * count;
        camera_component->SetViewMatrix(camera_position, mesh_center);
        camera_component->UpdateViewProjectionMatrix();

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }
        std::cout << ".";

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}

int test_purecolor_camera_loadmodel() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);

    std::fstream file_stream(target_mesh_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_mesh_file);
        return -1;
    }
    mesh_filter_component->LoadMesh(file_stream);

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kPureColorMaterial);
    auto pure_color_mat = std::dynamic_pointer_cast<PureColorMaterial>(mat);
    pure_color_mat->LoadAllContent();

    // 3. default env_light
    auto component1 = ComponentFactory::Instance().Create(kEnvironmentalLight);
    auto env_light_component = std::dynamic_pointer_cast<EnvironmentalLightComponent>(component1);
    // cube map
    std::shared_ptr<glit::GLTextureInfo> texture;
    texture = glit::GLTextureInfo::LoadCubeTexture(
        "backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin", 256);
    env_light_component->SetCubeMap(texture->texture, texture->width, texture->height);
    // panorama
    texture = glit::GLTextureInfo::LoadPanorama(
        "backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin");
    env_light_component->SetPanorama(texture->texture, texture->width, texture->height);
    // sph
    auto sph = glit::GLTextureInfo::LoadSphericalHarmonics(
        "backgroundCubes/01_attic_room_with_windows/diffuse_sph.json");
    env_light_component->SetDiffuseSphericalHarmonics(sph.first, sph.second);
    // brdf
    texture = glit::GLTextureInfo::LoadIntegratedBRDF(
        "backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin");
    env_light_component->SetIntegratedBRDF(texture->texture);

    // 4. default camera
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);
    camera_component->SetProjectionMatrix(
        kCameraType::kPerspective, glm::radians(45.0f),
        glit::cpp::CustomGLFWContext::GetCurrentGLFWContext()->GetAspect(), 0.2, 2000.0);
    glm::vec3 center = mesh_filter_component->GetBBox().GetCenter();
    // glm::vec3 camera_position = mesh_filter_component->GetBBox().GetCenter() +
    // mesh_filter_component->GetBBox().GetRadius() * 2.5f;
    // camera_component->SetViewMatrix(camera_position, center);

    // 5. renderable
    camera_component->AppendRenderables(
        mesh_filter_component->GetMesh(), pure_color_mat,
        mesh_filter_component->GetMesh()->submesh.front(), env_light_component,
        std::set<std::shared_ptr<PunctualLightComponent>>(),
        std::set<std::shared_ptr<DirectionalLightComponent>>(),
        std::set<std::shared_ptr<PointLightComponent>>(),
        std::set<std::shared_ptr<SpotLightComponent>>(), glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = pure_color_mat->GenerateVertexShaderKey(item);
        auto fskey = pure_color_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camera_position(0.0f, 0.0f, 2.5f);
        camera_position.x = mesh_filter_component->GetBBox().GetCenter().x +
                            mesh_filter_component->GetBBox().GetRadius().x * 2 + 0.01f * count;
        camera_component->SetViewMatrix(camera_position, center);
        camera_component->UpdateViewProjectionMatrix();

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }
        std::cout << ".";

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}

int test_pbr_camera_loadmodel() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);

    std::fstream file_stream(target_mesh_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_mesh_file);
        return -1;
    }
    mesh_filter_component->LoadMesh(file_stream);

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kBlinnPhongMaterial);
    auto single_mat = std::dynamic_pointer_cast<SingleMaterial>(mat);
    single_mat->LoadAllContent();

    // 3. default env_light
    auto component1 = ComponentFactory::Instance().Create(kEnvironmentalLight);
    auto env_light_component = std::dynamic_pointer_cast<EnvironmentalLightComponent>(component1);
    // cube map
    std::shared_ptr<glit::GLTextureInfo> texture;
    texture = glit::GLTextureInfo::LoadCubeTexture(
        "backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin", 256);
    env_light_component->SetCubeMap(texture->texture, texture->width, texture->height);
    // panorama
    texture = glit::GLTextureInfo::LoadPanorama(
        "backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin");
    env_light_component->SetPanorama(texture->texture, texture->width, texture->height);
    // sph
    auto sph = glit::GLTextureInfo::LoadSphericalHarmonics(
        "backgroundCubes/01_attic_room_with_windows/diffuse_sph.json");
    env_light_component->SetDiffuseSphericalHarmonics(sph.first, sph.second);
    // brdf
    texture = glit::GLTextureInfo::LoadIntegratedBRDF(
        "backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin");
    env_light_component->SetIntegratedBRDF(texture->texture);

    // 4. default camera
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);
    camera_component->SetProjectionMatrix(
        kCameraType::kPerspective, glm::radians(45.0f),
        glit::cpp::CustomGLFWContext::GetCurrentGLFWContext()->GetAspect(), 0.2, 2000.0);
    glm::vec3 center = mesh_filter_component->GetBBox().GetCenter();
    // glm::vec3 camera_position = mesh_filter_component->GetBBox().GetCenter() +
    // mesh_filter_component->GetBBox().GetRadius() * 2.5f;
    // camera_component->SetViewMatrix(camera_position, center);

    // 5. renderable
    camera_component->AppendRenderables(
        mesh_filter_component->GetMesh(), single_mat,
        mesh_filter_component->GetMesh()->submesh.front(), env_light_component,
        std::set<std::shared_ptr<PunctualLightComponent>>(),
        std::set<std::shared_ptr<DirectionalLightComponent>>(),
        std::set<std::shared_ptr<PointLightComponent>>(),
        std::set<std::shared_ptr<SpotLightComponent>>(), glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = single_mat->GenerateVertexShaderKey(item);
        auto fskey = single_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    float theta = 0;
    glm::vec3 camera_start_relative_position(0.0f, 0.0f,
                                             mesh_filter_component->GetBBox().GetRadius().z * 18);
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        theta += 0.01f;
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, theta, glm::vec3(0.0, 1.0, 0.0));
        glm::vec3 camera_position = trans * glm::vec4(camera_start_relative_position, 1.0f);
        camera_position += mesh_filter_component->GetBBox().GetCenter();
        camera_component->SetViewMatrix(camera_position, center);
        camera_component->UpdateViewProjectionMatrix();

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }
        std::cout << ".";

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}

int test_uvlayout_camera_loadmodel() {
    // try explain the pipeline without any entity

    // 1. mesh
    auto component0 = ComponentFactory::Instance().Create(kMeshFilter);
    auto mesh_filter_component = std::dynamic_pointer_cast<MeshFilterComponent>(component0);

    std::fstream file_stream(target_mesh_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_mesh_file);
        return -1;
    }
    mesh_filter_component->LoadMesh(file_stream);

    // 2. material
    auto mat = MaterialFactory::Instance().Create(kUVLayoutMaterial);
    auto single_mat = std::dynamic_pointer_cast<SingleMaterial>(mat);
    single_mat->LoadAllContent();

    // 3. default env_light
    auto component1 = ComponentFactory::Instance().Create(kEnvironmentalLight);
    auto env_light_component = std::dynamic_pointer_cast<EnvironmentalLightComponent>(component1);
    std::shared_ptr<glit::GLTextureInfo> texture;
    // cube map
    texture = glit::GLTextureInfo::LoadCubeTexture(
        "backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin", 256);
    env_light_component->SetCubeMap(texture->texture, texture->width, texture->height);
    // panorama
    texture = glit::GLTextureInfo::LoadPanorama(
        "backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin");
    env_light_component->SetPanorama(texture->texture, texture->width, texture->height);
    // sph
    auto sph = glit::GLTextureInfo::LoadSphericalHarmonics(
        "backgroundCubes/01_attic_room_with_windows/diffuse_sph.json");
    env_light_component->SetDiffuseSphericalHarmonics(sph.first, sph.second);
    // brdf
    texture = glit::GLTextureInfo::LoadIntegratedBRDF(
        "backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin");
    env_light_component->SetIntegratedBRDF(texture->texture);

    // 4. default camera
    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);
    camera_component->SetProjectionMatrix(
        kCameraType::kPerspective, glm::radians(45.0f),
        glit::cpp::CustomGLFWContext::GetCurrentGLFWContext()->GetAspect(), 0.2, 2000.0);
    glm::vec3 center = mesh_filter_component->GetBBox().GetCenter();
    // glm::vec3 camera_position = mesh_filter_component->GetBBox().GetCenter() +
    // mesh_filter_component->GetBBox().GetRadius() * 2.5f;
    // camera_component->SetViewMatrix(camera_position, center);

    // 5. renderable
    camera_component->AppendRenderables(
        mesh_filter_component->GetMesh(), single_mat,
        mesh_filter_component->GetMesh()->submesh.front(), env_light_component,
        std::set<std::shared_ptr<PunctualLightComponent>>(),
        std::set<std::shared_ptr<DirectionalLightComponent>>(),
        std::set<std::shared_ptr<PointLightComponent>>(),
        std::set<std::shared_ptr<SpotLightComponent>>(), glm::mat4(1.0), 0.0, false);
    {
        // inspect
        const RenderableItem& item = camera_component->GetOpaqueList().front();

        auto vskey = single_mat->GenerateVertexShaderKey(item);
        auto fskey = single_mat->GenerateFragmentShaderKey(item);

        LOG_INFO("Material: %s, vskey %lu, fskey %lu", mat->GetTypeStr().c_str(), vskey, fskey);
    }

    int count = 0;
    float theta = 0;
    glm::vec3 camera_start_relative_position(0.0f, 0.0f,
                                             mesh_filter_component->GetBBox().GetRadius().z * 18);
    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        theta += 0.01f;
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, theta, glm::vec3(0.0, 1.0, 0.0));
        glm::vec3 camera_position = trans * glm::vec4(camera_start_relative_position, 1.0f);
        camera_position += mesh_filter_component->GetBBox().GetCenter();
        camera_component->SetViewMatrix(camera_position, center);
        camera_component->UpdateViewProjectionMatrix();

        for (const auto& item : camera_component->GetOpaqueList()) {
            glit::RenderItem_V1(camera_component, item);
        }
        std::cout << ".";

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames);

    return 0;
}
