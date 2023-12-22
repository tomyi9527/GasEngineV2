#include "build_scene.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "FreeImage.h"
#include "basic_config.h"
#include "data_types/material/blinn_phong_material.h"
#include "data_types/material/dielectric_material.h"
#include "data_types/material/pure_color_material.h"
#include "data_types/material/skybox_material.h"
#include "data_types/material/xy_only_pure_color_material.h"
#include "data_types/material_factory.h"
#include "data_types/mesh_loader.h"
#include "ecs/component/animator_component.h"
#include "ecs/component/camera_component.h"
#include "ecs/component/environmental_light_component.h"
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component_factory.h"
#include "ecs/enhancement/skeleton_manager.h"
#include "ecs/entity/scene.h"
#include "ecs/loader/gas2_loader.h"
#include "ecs/skybox_entity.h"
#include "manipulator/orbit_manipulator.h"
#include "manipulator/simple_rotate_manipulator.h"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/opengl_interface.h"
#include "opengl/renderable_item.h"
#include "utils/encoding_conv.h"
#include "utils/gif_maker.h"
#include "utils/logger.h"
#include "utils/set_locale.h"
#include "utils/string_util.h"
#include "utils/time_stamp.h"
#include "ver.h"

bool CheckVersion(const std::string& resource_name);
void AddDefaultEnvLight(const std::shared_ptr<Scene>& scene);
void AddDefaultSkyBox(const std::shared_ptr<Scene>& scene, int mode);
void AddDefaultCamera(const std::shared_ptr<Scene>& scene,
                      const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                      Manipulator* manipulator = nullptr);

std::string ScreenShotToMemory(const RGBAImageData& data, const BasicConfig& config);

std::string ScreenAnimationToMemory(const std::vector<RGBAImageData>& frames, int fps,
                                    const BasicConfig& config);

// # FileCheck and DefaultSceneBuild
// check convertedFiles
bool CheckVersion(const std::string& resource_name) {
    std::string content = resource::ResourceManager::Instance().Load(resource_name);
    rapidjson::Document doc;
    doc.Parse(content.data(), content.size());
    if (!doc.IsObject()) {
        LOG_ERROR("input .convertedFiles file has malformed json content");
        return false;
    }
    auto it = doc.FindMember("version");
    if (it == doc.MemberEnd()) {
        LOG_ERROR("input .convertedFiles file doesn't have 'version' field");
        return false;
    }
    if (json::ToString(it->value) != "gas2") {
        LOG_ERROR("input .convertedFiles file 'version' is not 'gas2'");
        return false;
    }
    return true;
}

// default env_light
void AddDefaultEnvLight(const std::shared_ptr<Scene>& scene) {
    auto component = ComponentFactory::Instance().Create(kEnvironmentalLight);
    auto env_light_component = std::dynamic_pointer_cast<EnvironmentalLightComponent>(component);
    std::shared_ptr<glit::GLTextureInfo> texture;
#ifdef __APPLE__
    bool specular_use_panorama = true;
#else
    bool specular_use_panorama = false;
#endif
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
}

// default skybox
void AddDefaultSkyBox(const std::shared_ptr<Scene>& scene, int mode) {
    auto skybox_entity = SkyboxEntityFactory::Instance().GenerateSkybox("default_skybox");
    auto skybox_renderer_component = std::dynamic_pointer_cast<MeshRendererComponent>(
        skybox_entity->GetComponent(kMeshRenderer));
    auto skybox_mat = std::dynamic_pointer_cast<SkyBoxMaterial>(
        skybox_renderer_component->GetMaterials().front());
    if (mode == 0) {
        skybox_mat->ApplyPresetCubeMap();
    } else if (mode == 1) {
        skybox_mat->ApplyPresetAmbient();
    } else if (mode == 2) {
        skybox_mat->ApplyPresetImage();
    } else {
        skybox_mat->ApplyPresetSolidColor();
    }
    scene->AppendEntityToRoot(skybox_entity);
}

// attach default camera
void AddDefaultCamera(const std::shared_ptr<Scene>& scene,
                      const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                      Manipulator* manipulator) {
    float radius = scene->root->bbox.GetRadiusScalar();
    float far_plane = radius * 10.0;
    far_plane = far_plane < 999999.0 ? far_plane : 999999.0;
    float near_plane = (far_plane / 1000.0 < 0.001) ? 0.001 : (far_plane / 1000.0);
    float fov_radians = glm::radians(60.0f);

    auto component = ComponentFactory::Instance().Create(kCamera);
    auto camera_component = std::dynamic_pointer_cast<CameraComponent>(component);
    camera_component->SetProjectionMatrix(kCameraType::kPerspective, fov_radians,
                                          context->GetAspect(), near_plane, far_plane);
    context->on_resize = [camera_component, fov_radians, near_plane, far_plane](
                             std::shared_ptr<glit::cpp::CustomGLFWContext> window_context,
                             int width, int height) {
        camera_component->SetProjectionMatrix(kCameraType::kPerspective, fov_radians,
                                              window_context->GetAspect(), near_plane, far_plane);
    };

    glm::vec3 center = glm::vec3(0.0f);
    glm::vec3 camera_position = center;
    if (scene->root->bbox.IsValid()) {
        center = scene->root->bbox.GetCenter();
        camera_position = center;
    }
    camera_position[2] += std::max(1.0f, radius * 2.5f);
    camera_component->SetViewMatrix(camera_position, center);
    camera_component->UpdateViewProjectionMatrix();

    auto camera_entity = EntityFactory::Instance().CreateEntity();
    camera_entity->AddComponent(camera_component);
    scene->AppendEntityToRoot(camera_entity);

    // init a manipulator
    if (manipulator) manipulator->InitialSettings(camera_component, center, radius);
}

// bounding box related
pEntity CreateAsEntity(const Box& box, const glm::vec4& color, const pEntity& parent = nullptr) {
    auto e = EntityFactory::Instance().CreateEntity(parent);
    e->is_helper = true;
    pComponent ptr;
    ptr = ComponentFactory::Instance().Create(kMeshFilter);
    auto mf = std::dynamic_pointer_cast<MeshFilterComponent>(ptr);
    auto obj = OBJECT::GenerateOBJECT();
    obj->position.resize(box.vertices.size() * 3);
    for (int i = 0; i < box.vertices.size(); ++i) {
        obj->position[3 * i + 0] = box.vertices[i][0];
        obj->position[3 * i + 1] = box.vertices[i][1];
        obj->position[3 * i + 2] = box.vertices[i][2];
    }
    obj->section_item_map.emplace(kPOSITION, SECTION_ITEM());
    obj->topology.resize(box.edges.size() * 2);
    for (int i = 0; i < box.edges.size(); ++i) {
        obj->topology[2 * i + 0] = box.edges[i][0];
        obj->topology[2 * i + 1] = box.edges[i][1];
    }
    obj->section_item_map.emplace(kTOPOLOGY, SECTION_ITEM());
    obj->submesh.push_back(SUBMESH());
    mf->SetMesh(obj);
    mf->UpdateBBox();
    e->AddComponent(mf);
    ptr = ComponentFactory::Instance().Create(kMeshRenderer);
    auto mr = std::dynamic_pointer_cast<MeshRendererComponent>(ptr);
    auto mat = MaterialFactory::Instance().Create(kMaterialType::kPureColorMaterial);
    auto pcmat = std::dynamic_pointer_cast<PureColorMaterial>(mat);
    pcmat->SetDefaultColor(color);
    mat->SetWireFrame(true);
    mr->AddMaterial(mat);
    e->AddComponent(mr);
    obj->SubmitToOpenGL();
    return e;
}

std::vector<std::function<void()>> AddBBOXEntity(const pScene& scene) {
    if (!scene || !scene->root) {
        return {};
    }
    std::vector<pEntity> new_entity;
    std::vector<std::function<void()>> new_rule;
    s_Traverse(scene->root, nullptr, [&new_entity, &new_rule](const pEntity& e) {
        if (!e->bbox.IsValid() || e->is_helper) {
            return;
        }
        Box box;
        box.FromAABB(e->bbox);
        glm::vec4 color;
        if (e->HasComponent(kMeshFilter)) {
            color = glm::vec4(1.0f, 0.2f, 0.3f, 1.0f);
        } else {
            color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
        auto new_e = CreateAsEntity(box, color, nullptr);
        new_e->name =
            "GASEngine_inner_temp_object_" + std::to_string(reinterpret_cast<uint64_t>(&*new_e));
        new_entity.push_back(new_e);
        new_rule.push_back([new_e, e]() {
            Box box;
            box.FromAABB(e->bbox);
            auto ptr =
                std::dynamic_pointer_cast<MeshFilterComponent>(new_e->GetComponent(kMeshFilter));
            auto obj = ptr->GetMesh();
            obj->position.resize(box.vertices.size() * 3);
            for (int i = 0; i < box.vertices.size(); ++i) {
                obj->position[3 * i + 0] = box.vertices[i][0];
                obj->position[3 * i + 1] = box.vertices[i][1];
                obj->position[3 * i + 2] = box.vertices[i][2];
            }
            obj->topology.resize(box.edges.size() * 2);
            for (int i = 0; i < box.edges.size(); ++i) {
                obj->topology[2 * i + 0] = box.edges[i][0];
                obj->topology[2 * i + 1] = box.edges[i][1];
            }
            new_e->matrix_world = e->GetWorldMatrix();
            obj->UpdateToOpenGL();
        });
    });
    for (const auto& m : new_entity) {
        scene->root->AddChild(m);
    }
    new_entity.clear();
    return new_rule;
}

void RemoveBBOXEntity(const pScene& scene) {
    if (!scene || !scene->root) {
        return;
    }
    std::vector<pEntity> remove_entity;
    for (const auto& m : scene->root->children) {
        if (StartsWith(m->name, "GASEngine_inner_temp_object_")) {
            remove_entity.push_back(m);
        }
    }
    for (const auto& m : remove_entity) {
        scene->root->RemoveChild(m);
    }
    remove_entity.clear();
}

// # FileMemoryGeneration and SaveMethod
// to local file
void SaveMemoryToLocal(const BasicConfig& config, const std::string& memory,
                       const std::string& force_ext) {
    std::filesystem::path output_path = config.output;
    if (!force_ext.empty()) {
        if (output_path.has_extension()) {
            std::string ext = output_path.extension().string();
            for (auto& c : ext) {
                c = std::tolower(c);
            }
            if (ext != force_ext) output_path.replace_extension(force_ext);
        } else {
            output_path += force_ext;
        }
    }
    std::string output_string = output_path.string();
    std::fstream filestream(output_string, std::ios::binary | std::ios::out);
    if (filestream) {
        filestream.write(memory.data(), memory.size());
    }
}  // can add more function like this.

// outputs png file format memory content
std::string ScreenShotToMemory(const RGBAImageData& data, const BasicConfig& config) {
    FIBITMAP* bitmap = FreeImage_Allocate(data.width, data.height, 32);
    auto bytes = (RGBQUAD*)FreeImage_GetBits(bitmap);
    // 要注意skybox输出的透明度通道均为0，因此背景是透明的。
    // 若想要显示背景，将透明度通道设为255即可。
    for (int idx = 0; idx < data.height * data.width; ++idx) {
        bytes[idx].rgbRed = data.data[idx * 4 + 0];    // r
        bytes[idx].rgbGreen = data.data[idx * 4 + 1];  // g
        bytes[idx].rgbBlue = data.data[idx * 4 + 2];   // b
        bytes[idx].rgbReserved = data.data[idx * 4 + 3];  // a
    }
    if (data.width != config.window_width || data.height != config.window_height) {
        auto old_bitmap = bitmap;
        bitmap = FreeImage_Rescale(old_bitmap, config.window_width, config.window_height,
                                   FILTER_BILINEAR);
        FreeImage_Unload(old_bitmap);
    }

    auto memory = FreeImage_OpenMemory();
    FreeImage_SaveToMemory(FIF_PNG, bitmap, memory, 0);
    BYTE* mem_buffer = NULL;
    DWORD size_in_bytes = 0;

    FreeImage_AcquireMemory(memory, &mem_buffer, &size_in_bytes);  // return internal pointer
    // save the buffer in a file stream
    std::string memory_of_file;
    memory_of_file.assign(reinterpret_cast<char*>(mem_buffer), size_in_bytes);

    FreeImage_CloseMemory(memory);
    FreeImage_Unload(bitmap);
    return memory_of_file;
}

// outputs gif file format memory content
std::string ScreenAnimationToMemory(const std::vector<RGBAImageData>& frames, int fps,
                                    const BasicConfig& config) {
    LOG_INFO("making gif image...");
    GifMakerConfig cfg;
    cfg.height = config.window_height;
    cfg.width = config.window_width;
    cfg.fps = fps;
    GifMaker g;
    int64_t start = GetTimeStampInMs();
    g.Init(cfg);
    for (const auto& m : frames) {
        g.AddFrame(m, true, FREE_IMAGE_QUANTIZE::FIQ_NNQUANT);
    }
    int64_t middle = GetTimeStampInMs();
    auto mem = g.SaveToMem();
    LOG_INFO("gif image saved");
    int64_t end = GetTimeStampInMs();
    LOG_DEBUG("time used: 1st,  2nd,  total");
    LOG_DEBUG("time used: %lld, %lld, %lld", middle - start, end - middle, end - start);
    return mem;
}

// # run main loop or main procedure
int RunViewer(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
              const BasicConfig& config) {
    // create scene
    auto gas2_loader = GAS2Loader::GenerateLoader("");
    pScene scene = gas2_loader.Load(config.input);
    if (scene == nullptr) {
        LOG_ERROR("scene failed to generate, input is %s", config.input.c_str());
        return -1;
    }

    // add env light to scene
    AddDefaultEnvLight(scene);

    // add skybox to scene
    AddDefaultSkyBox(scene, config.background_type);

    // add env light to scene
    OrbitManipulator manipulator;
    AddDefaultCamera(scene, context, &manipulator);

    // use mouse input
    OrbitManipulatorStandardMouseKeyBoardController controller(manipulator);
    controller.BindGLFWContext(context);
    manipulator.Update(0.0f);

    // 打印信息
    scene->root->Print(std::cout);

    // update and render
    float delta = 0.02f;
    int64_t current_time_stamp = GetTimeStampInMs();
    int64_t last_time_stamp = GetTimeStampInMs();
    // AppGlobalResource::Instance().SetEnableMorphAnimation(false);
    // AppGlobalResource::Instance().SetEnableSkinning(false);
    // AppGlobalResource::Instance().SetEnableMotionBuilderParameters(false);
    //if (scene->GetActiveAnimationTotalDuration() > 10) {
    //    scene->SetAnimationPlaySpeed(scene->GetActiveAnimationTotalDuration() / 10.0);
    //}
    scene->SetAnimationPlaySpeed(config.play_speed);
    if (scene->GetActiveAnimator()) scene->GetActiveAnimator()->PlayFirst();
    scene->UpdateGlobalSettings();
    scene->Update(0.0f);
    std::vector<std::function<void()>> updater;
    if (config.draw_bbox) updater = AddBBOXEntity(scene);

    SkeletonManager::Instance().GetEntity()->enable = config.draw_skeleton;

    while (!glfwWindowShouldClose(context->window)) {
        delta = std::max((current_time_stamp - last_time_stamp) / 1000.0f, 0.002f);
        last_time_stamp = current_time_stamp;
        current_time_stamp = GetTimeStampInMs();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->Cull();
        if (scene->GetCameraCount() > 0) {
            auto current_camera = *(scene->GetCameras().begin());
            glit::RenderPBR_V1({current_camera});
        }
        manipulator.Update(delta);
        for (const auto& m : updater) {
            m();
        }
        scene->UpdateGlobalSettings();
        scene->Update(delta);
        SkeletonManager::Instance().Update();
        glfwSwapBuffers(context->window);
        glfwPollEvents();
    }
    if (config.draw_bbox) RemoveBBOXEntity(scene);
    return 0;
}

RGBAImageData GetBackgroundImage(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                                 const BasicConfig& config) {
    pScene scene = Scene::GenerateObject();

    // add skybox to scene
    AddDefaultSkyBox(scene, config.background_type);

    // add env light to scene
    SimpleRotateManipulator manipulator;
    AddDefaultCamera(scene, context, &manipulator);
    manipulator.Update(0.0f);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene->UpdateGlobalSettings();
    scene->Update(0);
    scene->Cull();
    if (scene->GetCameraCount() > 0) {
        auto current_camera = *(scene->GetCameras().begin());
        glit::RenderPBR_V1({current_camera});
    }
    return GetFrameBufferRGBAImageData(context);
}

int RunScreenShot(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                  const BasicConfig& config, ScreenShotSaver proc) {
    auto background_image = GetBackgroundImage(context, config);
    // create scene
    glfwSwapInterval(0);
    auto gas2_loader = GAS2Loader::GenerateLoader("");
    pScene scene = gas2_loader.Load(config.input);
    if (scene == nullptr) {
        LOG_ERROR("scene failed to generate, input is %s", config.input.c_str());
        return -1;
    }

    // add env light to scene
    AddDefaultEnvLight(scene);

    // add skybox to scene
    AddDefaultSkyBox(scene, config.background_type);

    // add env light to scene
    SimpleRotateManipulator manipulator;
    AddDefaultCamera(scene, context, &manipulator);
    manipulator.Update(0.0f);

    // setting flags
    float duration = 0.0f;
    float play_start = std::max(0.0f, config.play_start);
    float play_end = config.play_end;
    scene->SetAnimationPlaySpeed(config.play_speed);
    float play_speed = scene->GetAnimationPlaySpeed();
    int fps = std::clamp(config.fps, 1, 40);
    float time_step = 1.0f / fps;
    if (scene->HasAnimator()) {
        duration = scene->GetActiveAnimationTotalDuration() / play_speed;
        scene->SetAnimationPlayMode(kAnimationPlayMode::kAnimation_Play);
        scene->SetAnimationLoopMode(kAnimationLoopMode::kAnimation_Loop);
        scene->GetActiveAnimator()->PlayFirst();
    }
    scene->SetActiveAnimationProgress(play_start / duration);
    if (play_start > duration) {
        play_start = std::max(0.0f, duration - config.min_duration);
    }
    if (play_end < 0.0f) {
        play_end = duration;
    } else {
        play_end = std::min(std::max(play_start + config.min_duration, play_end), duration);
    }
    float play_duration = play_end - play_start;
    int total_frames = std::ceil(play_duration * fps);
    if (!config.save_gif) {
        total_frames = 1;
    } else {
        total_frames = std::clamp(total_frames, 1, 1000);
    }
    float rotate_step = 2 * M_PI / total_frames;

    RGBAImageData first_frame;
    GifMaker g;
    if (total_frames > 1) {
        GifMakerConfig cfg;
        cfg.async = true;
        cfg.pool_size = config.worker;
        cfg.height = config.window_height;
        cfg.width = config.window_width;
        cfg.SetFrameRate(config.fps);
        cfg.disposal = 3;  // dispose PreviousFrame
        g.Init(cfg);
        // disable background optimization
        // g.AddBackground(background_image);  // together with disposal = 3, discard_alpha = false,
                                            // can reduce img size significantly
    }

    // update and render
    scene->UpdateGlobalSettings();
    scene->Update(0.0f);
    std::vector<std::function<void()>> updater;
    if (config.draw_bbox) updater = AddBBOXEntity(scene);
    SkeletonManager::Instance().GetEntity()->enable = config.draw_skeleton;

    // std::vector<RGBAImageData> frames;
    for (int current_frame = 0; current_frame < total_frames; ++current_frame) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->Cull();
        if (scene->GetCameraCount() > 0) {
            auto current_camera = *(scene->GetCameras().begin());
            glit::RenderPBR_V1({current_camera});
        }
        if (config.rotate_snap) manipulator.Update(rotate_step);
        for (const auto& m : updater) {
            m();
        }
        scene->UpdateGlobalSettings();
        scene->Update(time_step);
        SkeletonManager::Instance().Update();
        // glfwSwapBuffers(context->window);
        if (current_frame == 0) {
            first_frame = GetFrameBufferRGBAImageData(context);
            g.AddFrame(first_frame, false, FREE_IMAGE_QUANTIZE::FIQ_NNQUANT); // use transparent background
        } else {
            g.AddFrame(GetFrameBufferRGBAImageData(context), false, FREE_IMAGE_QUANTIZE::FIQ_NNQUANT); // use transparent background
        }
        // frames.push_back(GetFrameBufferRGBAImageData(context));
        glfwPollEvents();
    }
    if (config.draw_bbox) RemoveBBOXEntity(scene);

    // output
    std::string output_type;
    std::string file_content;
    if (total_frames > 1 && config.save_gif) {
        output_type = ".gif";
        // file_content = ScreenAnimationToMemory(frames, fps, config);
        file_content = g.SaveToMem();
    } else if (total_frames == 1) {
        output_type = ".png";
        file_content = ScreenShotToMemory(first_frame, config);
    } else {
        LOG_ERROR("no frame is captured");
    }

    if (!file_content.empty() && proc) {
        proc(config, file_content, output_type);
    }

    return 0;
}

ProgramInformation ProgramInformation::instance;

ProgramInformation::ProgramInformation() {
#ifdef FORCE_PRINT_GIT_INFO
    std::cerr << "current program commit hash: " << commit_hash << std::endl;
    std::cerr << "committed at: " << commit_time << " by " << commit_author << std::endl;
    std::cerr << "bin built at: " << build_time << std::endl;
    if (std::string(commit_tag).find("NOTFOUND") == std::string::npos) {
        std::cerr << "current program commit tag:" << commit_tag << std::endl;
    }
    std::cerr << "==========================================================================="
              << std::endl;
#endif
}