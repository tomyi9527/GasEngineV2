#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "ecs/entity/scene.h"
#include "ecs/entity_factory.h"
#include "ecs/loader/gas2_loader.h"
#include "opengl/opengl_interface.h"
#include "utils/logger.h"

std::string GetSampleStructureJson();

int main() {
    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;

    glit::Init();
    {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->CreateContext();
        std::stringstream ss(GetSampleStructureJson());

        auto loader = GAS2Loader::GenerateLoader("");
        pScene scene = loader.Load(ss);

        if (scene == nullptr) {
            return -1;
        }

        scene->root->Print(std::cout);
    }
    glit::Terminate();

    return 0;
}

std::string GetSampleStructureJson() {
    return R"sample_json({
    "version":5,
    "srcVersion":"FBX",
    "name":"girlwalk.fbx",
    "setting":{},
    "nodeTree":
    {
        "uniqueID":9,
        "guid":"92054223-cb8c-4d43-8f61-e56d62da4d40",
        "name":"RootNode",
        "translation":[0.000000,0.000000,0.000000],
        "rotation":[0.000000,0.000000,0.000000],
        "scaling":[1.000000,1.000000,1.000000],
        "MB_PROPS":
        {
            "ScalingPivot":[0.000000,0.000000,0.000000],
            "ScalingOffset":[1.000000,0.000000,0.000000],
            "RotationPivot":[0.000000,0.000000,0.000000],
            "RotationOffset":[0.000000,0.000000,0.000000],
            "PreRotation":[0.000000,0.000000,0.000000],
            "PostRotation":[0.000000,0.000000,0.000000],
            "RotationOrder":"XYZ",
            "InheritType":"Local(RrSs)",
            "Visibility":1.000000,
            "VisibilityInheritance":true
        },
        "components":
        {
            "animator":
            {
                "syncLoading":false,
                "uniqueID":-1,
                "clips":
                [
                    "girlwalk.fbx.Take_001.546.animation.bin"
                ]
            }
        },
        "children":
        [
            {
                "uniqueID":163,
                "guid":"3b742ae1-141a-406d-bf4a-545b38434e1a",
                "name":"model_body",
                "translation":[0.000000,0.000000,0.000000],
                "rotation":[0.000000,0.000000,0.000000],
                "scaling":[1.000000,1.000000,1.000000],
                "MB_PROPS":
                {
                    "ScalingPivot":[0.000000,0.000000,0.000000],
                    "ScalingOffset":[0.000000,0.000000,0.000000],
                    "RotationPivot":[0.000000,0.000000,0.000000],
                    "RotationOffset":[0.000000,0.000000,0.000000],
                    "PreRotation":[0.000000,0.000000,0.000000],
                    "PostRotation":[0.000000,0.000000,0.000000],
                    "RotationOrder":"XYZ",
                    "InheritType":"Parent(RSrs)",
                    "Visibility":1.000000,
                    "VisibilityInheritance":true
                },
                "components":
                {
                    "meshFilter":
                    {
                        "syncLoading":false,
                        "uniqueID":326,
                        "mesh":"../../resources/model/girlwalk/gas2/girlwalk.fbx.model_body.326.mesh.bin",
                        "bbox":
                        [
                            [-7.739,0.100,-1.490],
                            [7.738,19.077,1.382]
                        ]
                    },
                    "meshRenderer":
                    {
                        "syncLoading":false,
                        "uniqueID":-1,
                        "materials":
                        [
                            "girlwalk.fbx.body_mat.285.mat.json"
                        ]
                    }
                }
            }
        ]
    }
})sample_json";
}