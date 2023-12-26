#pragma once
#include <memory>
#include "src/max_structure.h"
#include "src/max_parser.h"

std::unique_ptr<max::MaxCracker> LoadFromMax(const std::string& full_file_path);