#pragma once
#include <memory>
#include <string>
#include "basic_config.h"
#include "gflags/gflags.h"
#include "opengl/opengl_interface.h"

struct AppConfig : public BasicConfig {};

int LoadFromArg(int argc, char* argv[], AppConfig& output);