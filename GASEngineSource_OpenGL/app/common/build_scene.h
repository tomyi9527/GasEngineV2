#pragma once
#include <memory>
#include <vector>
#include "basic_config.h"
#include "opengl/opengl_interface.h"
#include "opengl/buffer_type.h"

#define FORCE_PRINT_GIT_INFO

class ProgramInformation {
  protected:
    static ProgramInformation instance;
    ProgramInformation(); // print some key information on initialization
};

bool CheckVersion(const std::string& resource_name);

// saver:
typedef void (*ScreenShotSaver)(const BasicConfig& config, const std::string& file_content, const std::string& file_ext);

void SaveMemoryToLocal(const BasicConfig& config, const std::string& memory,
                       const std::string& force_ext = ".png");

// main loop:
int RunScreenShot(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
                  const BasicConfig& config, ScreenShotSaver proc = SaveMemoryToLocal);
int RunViewer(const std::shared_ptr<glit::cpp::CustomGLFWContext>& context,
              const BasicConfig& config);

#ifdef WIN32
void WindowsSetNonWindowCrashHandler();
#endif
