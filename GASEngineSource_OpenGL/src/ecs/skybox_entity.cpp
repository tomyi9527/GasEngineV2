#include "skybox_entity.h"
#include "data_types/material/skybox_material.h"
#include "data_types/material_factory.h"
#include "opengl/opengl_interface.h"

std::shared_ptr<Entity> SkyboxEntityFactory::GenerateSkybox(const std::string& name) {
    // mesh_filter
    std::shared_ptr<OBJECT> sky_box_obj = OBJECT::GenerateOBJECT();
    sky_box_obj->position.assign({1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0});
    sky_box_obj->section_item_map[kPOSITION] = SECTION_ITEM();
    sky_box_obj->submesh.assign({SUBMESH(0, 4)});
    sky_box_obj->section_item_map[kSUBMESH] = SECTION_ITEM();
    sky_box_obj->draw_mode = GL_TRIANGLE_STRIP;
    // submit to gl
    sky_box_obj->SubmitToOpenGL();
    auto c_mesh_filter = std::dynamic_pointer_cast<MeshFilterComponent>(
        ComponentFactory::Instance().Create(kMeshFilter));
    c_mesh_filter->SetMesh(std::move(sky_box_obj));

    // mesh_renderer
    // create skybox Material
    std::shared_ptr<Material> mat = MaterialFactory::Instance().Create(kSkyboxMaterial);
    auto skybox_mat = std::dynamic_pointer_cast<SkyBoxMaterial>(mat);
    skybox_mat->ApplyPresetSolidColor();

    auto c_mesh_renderer = std::dynamic_pointer_cast<MeshRendererComponent>(
        ComponentFactory::Instance().Create(kMeshRenderer));
    c_mesh_renderer->AddMaterial(mat);

    // skybox entity
    auto entity = EntityFactory::Instance().CreateEntity();
    entity->name = name;
    entity->AddComponent(c_mesh_filter);
    entity->AddComponent(c_mesh_renderer);
    return entity;
}
