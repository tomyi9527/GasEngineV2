#pragma once
#include <memory>
#include <string>
#include "arthub_client.h"
#include "basic_config.h"
#include "gflags/gflags.h"
#include "opengl/opengl_interface.h"

static_assert(!glit::is_osmesa_build, "viewer must have a visible window, osmesa is not supported");

struct AppConfig : public BasicConfig {
    std::string token;
    NetworkConditionType network_env = kNetworkCondition_Normal;
};

int LoadFromArg(int argc, char* argv[], AppConfig& output);