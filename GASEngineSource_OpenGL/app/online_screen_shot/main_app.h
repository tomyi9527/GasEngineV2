#pragma once
#include <memory>
#include <string>
#include "arthub_client.h"
#include "basic_config.h"
#include "gflags/gflags.h"
#include "opengl/opengl_interface.h"

enum OutputMethod { kOutput_LocalFile = 0, kOutput_UpdateArthubPreview = 1, kOutput_Both = 2 };

struct AppConfig : public BasicConfig {
    std::string token;
    std::string pcg_token;
    NetworkConditionType network_env = kNetworkCondition_Normal;
    OutputMethod output_method = kOutput_LocalFile;
};

int LoadFromArg(int argc, char* argv[], AppConfig& output);
void SaveMemoryToArthub(const BasicConfig& config, const std::string& memory,
                        const std::string& force_ext);
void SaveMemoryToBoth(const BasicConfig& config, const std::string& memory,
                      const std::string& force_ext);