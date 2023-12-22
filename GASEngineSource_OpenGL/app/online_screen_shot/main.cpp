#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "build_scene.h"
#include "main_app.h"
#include "opengl/global_resource.h"
#include "opengl/opengl_interface.h"
#include "utils/encoding_conv.h"
#include "utils/env_handle.h"
#include "utils/logger.h"
#include "utils/set_locale.h"

#ifdef _MSC_VER
int wmain(int argc, wchar_t* argv[]) {
    WindowsSetNonWindowCrashHandler();
    SetLocale("");

    char** argv_utf8 = new char*[argc];
    std::vector<std::string> tempArray;
    for (int i = 0; i < argc; ++i) {
        std::wstring ws = argv[i];
        tempArray.push_back(UCS2ToUTF8(ws));
    }

    for (int i = 0; i < argc; ++i) {
        const char* p = tempArray[i].c_str();
        argv_utf8[i] = const_cast<char*>(p);
    }
#else
int main(int argc, char* argv[]) {
    char** argv_utf8 = argv;
#endif
    if (glit::is_osmesa_build &&
        (get_env("GALLIUM_DRIVER").empty() || get_env("MESA_GL_VERSION_OVERRIDE").empty() ||
         get_env("MESA_GLSL_VERSION_OVERRIDE").empty())) {
        // 此时的logger未初始化，将打印到console上
        LOG_ERROR("you can try setting following env for higher opengl version:");
        LOG_ERROR("LIBGL_ALWAYS_SOFTWARE=true    ");
        LOG_ERROR("GALLIUM_DRIVER=llvmpipe       ");
        LOG_ERROR("MESA_GL_VERSION_OVERRIDE=4.6  ");
        LOG_ERROR("MESA_GLSL_VERSION_OVERRIDE=460");
        LOG_ERROR("MESA_EXTENSION_OVERRIDE=GL_ARB_shader_texture_lod");
        exit(-1);
    }

    // load config
    AppConfig config;
    int code = LoadFromArg(argc, argv_utf8, config);  // argc, argv is empty after this.
    if (code != 0) {
        exit(code);
    }

    // initial
    InitL4CPLogger(config.logger_mode, config.log_level, config.log_file, config.locale_id);
    glit::Init();
    try {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->visible = false;
        context->name = config.input;
        if (glit::is_osmesa_build) {
            context->width = config.window_width * config.sampler;
            context->height = config.window_height * config.sampler;
        } else {
            context->width = config.window_width;
            context->height = config.window_height;
        }
        context->CreateContext(config.opengl_major, config.opengl_minor, glit::is_osmesa_build);
        if (context->window == nullptr) {
            LOG_ERROR("creation of gl context failed.");
        } else {
            AppGlobalResource::Instance().Init();
            // run
            ScreenShotSaver output_method = nullptr;
            if (config.output_method == kOutput_LocalFile) {
                output_method = SaveMemoryToLocal;
            } else if (config.output_method == kOutput_UpdateArthubPreview) {
                output_method = SaveMemoryToArthub;
            } else if (config.output_method == kOutput_Both) {
                output_method = SaveMemoryToBoth;
            }
            RunScreenShot(context, config, output_method);
            AppGlobalResource::Instance().Terminate();
        }
    } catch (std::exception& e) {
        std::string exception_name = e.what();
#if defined WIN32 && !defined UNICODE
        exception_name = Utf8ToGBK(exception_name);
#endif
        LOG_ERROR("exception: %s", exception_name.c_str());
        glit::Terminate();
        ShutDownL4CPLogger();
        exit(-1);
    }
    glit::Terminate();
    ShutDownL4CPLogger();
    return 0;
}