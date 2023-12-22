#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "ecs/component/mesh_filter_component.h"
#include "ecs/component_factory.h"
#include "ecs/entity/scene.h"
#include "ecs/entity_factory.h"
#include "opengl/opengl_interface.h"
#include "utils/logger.h"

constexpr char target_file[] =
    "../../resources/model/girlwalk/gas2/girlwalk.fbx.model_hair_inner.348.mesh.bin";

void test0();
void test1();
void test2();

int main() {
    glit::Init();
    {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->CreateContext();
        test0();
        test1();
        test2();
    }
    glit::Terminate();
    return 0;
}

constexpr int buckets = 4;
void test0() {
    std::vector<int> counter(buckets, 0);
    auto map_func = [](uint64_t index) { return (index / 16) % buckets; };
    std::set<pEntity> e_set;
    for (int i = 0; i < 36000; ++i) {
        auto ptr = EntityFactory::Instance().CreateEntity(nullptr);
        uint64_t ptr_value = (uint64_t) & (*ptr);
        ++counter[map_func(ptr_value)];
        e_set.emplace(ptr);
    }
    for (const auto& item : e_set) EntityFactory::Instance().Destroy(item);
    e_set.clear();
}

void test1() {
    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;
    auto root = EntityFactory::Instance().CreateEntity(nullptr);
    root->name = "root";
    auto model1 = EntityFactory::Instance().CreateEntity(root);
    model1->name = "model1";
    root->Print(std::cout);
    auto component1 = ComponentFactory::Instance().Create(kComponentType::kMeshFilter);

    auto mf_c1 = dynamic_cast<MeshFilterComponent*>(component1.get());
    std::fstream file_stream(target_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_file);
        return;
    }
    mf_c1->LoadMesh(file_stream);

    model1->AddComponent(std::move(component1));
    root->AddChild(model1);
    root->Print(std::cout);
    root->AddChild(model1);
    root->Print(std::cout);
    root->RemoveChild(model1);
    root->Print(std::cout);
}

void test2() {
    std::cout << "current working directory: " << std::filesystem::current_path() << std::endl;
    auto scene = Scene::GenerateObject("my-scene-1");

    auto model1 = EntityFactory::Instance().CreateEntity();
    model1->name = "model1";

    scene->AppendEntityToRoot(model1);
    scene->root->Print(std::cout);

    auto component1 = ComponentFactory::Instance().Create(kComponentType::kMeshFilter);

    auto mf_c1 = dynamic_cast<MeshFilterComponent*>(component1.get());
    std::fstream file_stream(target_file, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        LOG_ERROR("can not open file for read: %s", target_file);
        return;
    }
    mf_c1->LoadMesh(file_stream);

    model1->AddComponent(std::move(component1));
    scene->root->Print(std::cout);
}