#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "mesh_loader.h"
#include "utils/set_locale.h"

// constexpr char target_file[] =
// "../../resources/model/girlwalk/gas2/girlwalk.fbx.model_teeth_upper.328.mesh.bin"; constexpr char
// target_file[] =
// "../../resources/model/girlwalk/gas2/girlwalk.fbx.model_teeth_upper.328.mesh.bin";
//constexpr char target_file[] =
//    "../../resources/model/girlwalk/gas2/girlwalk.fbx.model_hair_inner.348.mesh.bin";
// constexpr char target_file[] =
// "../../resources/model/girlwalk/gas2/girlwalk.fbx.Take_001.546.animation.bin";
constexpr char target_file[] = "../../resources/model/jollybones/jollybones.fbx.All_Taffy1.77.mesh.bin";

int main() {
    SetLocale("zh_CN.UTF-8");
    // return 0;

    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;
    std::fstream file_stream(target_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_file);
        return -1;
    }
    OBJECT_LOADER loader;
    std::shared_ptr<OBJECT> obj = loader.LoadObject(file_stream);

    std::fstream file("test.obj", std::ios::out | std::ios::binary);
    obj->ExportAsObj(file);
    return 0;
}

// add this to start of test.obj
// ========================== test.obj
// mtllib test.mtl
// usemtl test

// add this file
// ========================== test.mtl
// newmtl test
// map_Ka body_diff.jpg