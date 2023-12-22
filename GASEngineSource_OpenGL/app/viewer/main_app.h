#pragma once
#include <memory>
#include <string>
#include "basic_config.h"
#include "gflags/gflags.h"
#include "opengl/opengl_interface.h"

static_assert(!glit::is_osmesa_build, "viewer must have a visible window, osmesa is not supported");

struct AppConfig : public BasicConfig {};

int LoadFromArg(int argc, char* argv[], AppConfig& output);