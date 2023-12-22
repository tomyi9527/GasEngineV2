#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include "build_scene.h"
#include "main_app.h"
#include "opengl/global_resource.h"
#include "opengl/opengl_interface.h"
#include "utils/env_handle.h"
#include "utils/logger.h"
#include "utils/set_locale.h"

int main(int argc, char* argv[]) {
    if (glit::is_osmesa_build &&
        (get_env("GALLIUM_DRIVER").empty() || get_env("MESA_GL_VERSION_OVERRIDE").empty() ||
         get_env("MESA_GLSL_VERSION_OVERRIDE").empty())) {
        // 此时的logger未初始化，将打印到console上
        LOG_ERROR("you can try setting following env for higher opengl version:");
        LOG_ERROR("LIBGL_ALWAYS_SOFTWARE=true    ");
        LOG_ERROR("GALLIUM_DRIVER=llvmpipe       ");
        LOG_ERROR("MESA_GL_VERSION_OVERRIDE=4.6  ");
        LOG_ERROR("MESA_GLSL_VERSION_OVERRIDE=460");
        exit(-1);
    }

    // load config
    AppConfig config;
    int code = LoadFromArg(argc, argv, config);
    if (code != 0) {
        exit(code);
    }

    // initial
    InitL4CPLogger(config.logger_mode, config.log_level, config.log_file, config.locale_id);
    glit::Init();
    {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->visible = true;
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
            RunViewer(context, config);
            AppGlobalResource::Instance().Terminate();
        }
    }
    glit::Terminate();
    ShutDownL4CPLogger();
    return 0;
}