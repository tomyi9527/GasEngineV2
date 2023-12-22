#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "data_types/keyframe_animator_loader.h"

constexpr char target_file[] =
    "../../resources/model/girlwalk/gas2/girlwalk.fbx.Take_001.546.animation.bin";

int main() {
    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;

    std::fstream file_stream(target_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_file);
        return -1;
    }
    KeyframeAnimationLoader loader;
    std::shared_ptr<KeyframeAnimation> obj = loader.LoadAnimation(file_stream);
    return 0;
}
