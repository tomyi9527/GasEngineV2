#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include "data_types/material/blinn_phong_material.h"
#include "data_types/material/pure_color_material.h"
#include "data_types/material/skybox_material.h"
#include "data_types/material/xy_only_pure_color_material.h"
#include "data_types/material_factory.h"
#include "data_types/mesh_loader.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component_factory.h"
#include "ecs/entity/scene.h"
#include "ecs/loader/gas2_loader.h"
#include "ecs/skybox_entity.h"
#include "manipulator/orbit_manipulator.h"
#include "manipulator/simple_rotate_manipulator.h"
#include "opengl/buffer_type.h"
#include "opengl/renderable_item.h"
#include "utils/bmp.h"
#include "utils/logger.h"
#include "utils/time_stamp.h"

constexpr char target_scene_file[] =
    "../../resources/model/girlwalk/gas2/girlwalk.fbx.structure.json";
// constexpr char target_scene_file[] = "../../resources/model/200w/200w.FBX.structure.json";

constexpr int total_time = 2;            // 2s
constexpr int frames = total_time * 60;  // per second

int test_pbr_camera_loadmodel(std::shared_ptr<glit::cpp::CustomGLFWContext> window_context);

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

    int start = GetTimeStampInMs();
    // 0. init
    glit::Init();

    {
        auto window_context = glit::cpp::CustomGLFWContext::Generate();
        window_context->width = 1000;
        window_context->height = 800;
        window_context->CreateContext(4, 6, osmesa);  // create opengl 4.6 core
        // glit::PrintExtensions();

        test_pbr_camera_loadmodel(window_context);
        SaveScreen(window_context->fb, window_context->width, window_context->height,
                   "test_pbr_camera_loadscene.bmp");
    }

    // final. clean
    glit::Terminate();

    int end = GetTimeStampInMs();
    std::cout << "total : " << end - start << std::endl;

    return 0;
}

inline void RenderScene(const std::shared_ptr<Scene>& scene) {
    scene->Cull();
    scene->UpdateGlobalSettings();
    scene->Update();
    if (scene->GetCameraCount() > 0) {
        auto current_camera = *(scene->GetCameras().begin());
        glit::RenderPBR_V1({current_camera});
    }
}

inline void RenderScene(const std::shared_ptr<Scene>& scene, const glm::mat4 view_matrix) {
    scene->Cull();
    scene->UpdateGlobalSettings();
    scene->Update();
    if (scene->GetCameraCount() > 0) {
        auto current_camera = *(scene->GetCameras().begin());
        current_camera->SetViewMatrix(view_matrix);

        glit::RenderPBR_V1({current_camera});
    }
}

int test_pbr_camera_loadmodel(std::shared_ptr<glit::cpp::CustomGLFWContext> window_context) {
    GLFWwindow* window = window_context->window;

    // 1. scene
    std::fstream file_stream(target_scene_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_scene_file);
        return -1;
    }
    auto gas2_loader =
        GAS2Loader::GenerateLoader(std::filesystem::path(target_scene_file).parent_path().string());
    pScene scene = gas2_loader.Load(file_stream);

    // 2. default env_light
    auto component1 = ComponentFactory::Instance().Create(kEnvironmentalLight);
    auto env_light_component = std::dynamic_pointer_cast<EnvironmentalLightComponent>(component1);
    std::shared_ptr<glit::GLTextureInfo> texture;
    bool specular_use_panorama = true;
    // sph
    auto sph = glit::GLTextureInfo::LoadSphericalHarmonics(
        "backgroundCubes/01_attic_room_with_windows/diffuse_sph.json");
    env_light_component->SetDiffuseSphericalHarmonics(sph.first, sph.second);
    // either cubemap or panorama needs to be set to compile a blinn-phong shader
    // brdf
    texture = glit::GLTextureInfo::LoadIntegratedBRDF(
        "backgroundCubes/01_attic_room_with_windows/brdf_ue4.bin");
    env_light_component->SetIntegratedBRDF(texture->texture);
    if (specular_use_panorama) {
        // panorama
        texture = glit::GLTextureInfo::LoadPanorama(
            "backgroundCubes/01_attic_room_with_windows/specular_panorama_ue4_1024_luv.bin");
        env_light_component->SetPanorama(texture->texture, texture->width, texture->height);
    } else {
        // cube map
        texture = glit::GLTextureInfo::LoadCubeTexture(
            "backgroundCubes/01_attic_room_with_windows/specular_cubemap_ue4_256_luv.bin", 256);
        env_light_component->SetCubeMap(texture->texture, texture->width, texture->height);
    }

    auto envlight_entity = EntityFactory::Instance().CreateEntity();
    envlight_entity->AddComponent(env_light_component);
    scene->AppendEntityToRoot(envlight_entity);

    // 4. default skybox
    auto skybox_entity = SkyboxEntityFactory::Instance().GenerateSkybox("default_skybox");
    auto skybox_mat = std::dynamic_pointer_cast<SkyBoxMaterial>(
        std::dynamic_pointer_cast<MeshRendererComponent>(skybox_entity->GetComponent(kMeshRenderer))
            ->GetMaterials()
            .front());
    // skybox_mat->ApplyPresetCubeMap();
    // skybox_mat->ApplyPresetAmbient();
    skybox_mat->ApplyPresetImage();
    scene->AppendEntityToRoot(skybox_entity);

    // 5. default camera
    float radius = scene->root->bbox.GetRadiusScalar();
    float far_plane = radius * 10.0;
    far_plane = far_plane < 99999.0 ? far_plane : 99999.0;
    float near_plane = (far_plane / 1000.0 < 0.001) ? 0.001 : (far_plane / 1000.0);
    float fov_radians = glm::radians(60.0f);

    auto component2 = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component2);
    camera_component->SetProjectionMatrix(kCameraType::kPerspective, fov_radians,
                                          window_context->GetAspect(), near_plane, far_plane);
    window_context->on_resize = [camera_component, fov_radians, near_plane, far_plane](
                                    std::shared_ptr<glit::cpp::CustomGLFWContext> window_context,
                                    int width, int height) {
        camera_component->SetProjectionMatrix(kCameraType::kPerspective, fov_radians,
                                              window_context->GetAspect(), near_plane, far_plane);
    };

    glm::vec3 center = scene->root->bbox.GetCenter();
    glm::vec3 camera_position = center;
    camera_position[2] += std::max(1.0f, scene->root->bbox.GetRadiusScalar() * 2.5f);
    camera_component->SetViewMatrix(camera_position, center);
    camera_component->UpdateViewProjectionMatrix();

    // bind a manipulator
    OrbitManipulator manipulator;
    manipulator.InitialSettings(camera_component, center, radius);
    OrbitManipulatorStandardMouseKeyBoardController controller(manipulator);
    controller.BindGLFWContext(window_context);

    SimpleRotateManipulator manipulator2;
    manipulator2.InitialSettings(camera_component, center, radius);

    auto camera_entity = EntityFactory::Instance().CreateEntity();
    camera_entity->AddComponent(camera_component);
    scene->AppendEntityToRoot(camera_entity);

    // update
    scene->UpdateGlobalSettings();
    scene->Update();

    int count = 0;
    float theta = 0;
    bool use_orbit_manipulator = false;

    do {
        ++count;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // theta += 0.01f;
        // glm::mat4 trans = glm::mat4(1.0f);
        // trans = glm::rotate(trans, theta, glm::vec3(0.0, 1.0, 0.0));
        // glm::vec3 camera_position = trans * glm::vec4(camera_start_relative_position, 1.0f);
        // camera_position += center;

        int start = GetTimeStampInMs();
        if (use_orbit_manipulator) {
            manipulator.Update(0.01f);
        } else {
            manipulator2.Update(0.01f);
        }
        RenderScene(scene);
        glfwSwapBuffers(window);
        int end = GetTimeStampInMs();

        // std::cout << "render one frame: " <<  end - start << std::endl;

        glfwPollEvents();
    } while (!glfwWindowShouldClose(window) && !osmesa && count < frames * 100);

    return 0;
}