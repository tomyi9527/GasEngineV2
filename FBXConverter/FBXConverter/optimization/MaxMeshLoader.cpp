#include "MaxMeshLoader.h"
#include "CommonStruct.h"
#include <assert.h>

using namespace max;
using namespace FBXSDK_NAMESPACE;

// WARNING: will only write material indices, material content is not set.
//          material is exported directly with StandardMaterial
bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh*, const max::EditablePoly & poly, const max::StandardMaterial& single, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);
bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh*, const max::EditablePoly & poly, const max::MultiMaterial& multi, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);
bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh*, const max::EditablePoly & poly, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);


FbxMesh * ToFbxMesh(const max::EditablePoly & poly, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager* _mgr, FBXSDK_NAMESPACE::FbxScene* myScene) {
    // 未应用uniqueID
    // uniqueID 存储在 user ptr
    const int32_t vertex_count = poly.points.size();
    FbxNode* node = FbxNode::Create(myScene, poly.mesh_name.c_str());
    FbxMesh* mesh = FbxMesh::Create(myScene, "");

    node->AddNodeAttribute(mesh);
    myScene->GetRootNode()->AddChild(node);

    // Create and fill in the vertex position data source.
    mesh->InitControlPoints(vertex_count);
    FbxVector4* ControlPoints = mesh->GetControlPoints();
    for (int32_t i = 0; i < vertex_count; ++i) {
        const auto& p = poly.points[i];
        ControlPoints[i].Set(p.x, p.y, p.z, p.reserved);
    }

    for (int i = 0; i < poly.faces.size(); ++i) {
        mesh->BeginPolygon();
        for (int j = 0; j < poly.faces[i].index.size(); ++j) {
            mesh->AddPolygon(poly.faces[i].index[j], -1);
        }
        mesh->EndPolygon();
    }

    static_assert(sizeof(void*) == 8, "should compile on 64-bit system");
    uint32_t id[2] = {poly.id, poly.parent_id};
    uint64_t id_encoded = *(uint64_t*)id;
    mesh->SetUserDataPtr((void*)id_encoded);
#if 1
    // To Decode id:
    uint64_t _decode_id_encoded = (uint64_t)mesh->GetUserDataPtr();
    uint32_t* _decode_id = (uint32_t*)&_decode_id_encoded; // size: 2
    assert(poly.id == id[0]);
    assert(poly.parent_id == id[1]);
#endif

    // Create Layer 0 to hold the normals
    FbxLayer* LayerZero = mesh->GetLayer(0);
    if (LayerZero == NULL) {
        mesh->CreateLayer();
        LayerZero = mesh->GetLayer(0);
    }

    if (!poly.normal_points.empty()) {
        // Create and fill in the per-face-vertex normal data source.
        // We extract the Z-tangent and drop the X/Y-tangents which are also stored in the render mesh.
        FbxLayerElementNormal* LayerElementNormal = FbxLayerElementNormal::Create(mesh, "");

        LayerElementNormal->SetMappingMode(FbxLayerElement::eByPolygonVertex);
        // Set the normal values for every control point.
        LayerElementNormal->SetReferenceMode(FbxLayerElement::eIndexToDirect);

        for (int32_t i = 0; i < poly.normal_points.size(); ++i) {
            const auto& v = poly.normal_points[i];
            FbxVector4 FbxNormal;
            FbxNormal.Set(v.x, v.y, v.z, v.reserved);

            LayerElementNormal->GetDirectArray().Add(FbxNormal);
        }
        for (int32_t face = 0; face < poly.normal_faces.size(); ++face) {
            for (int32_t j = 0; j < poly.normal_faces[face].index.size(); ++j) {
                LayerElementNormal->GetIndexArray().Add(poly.normal_faces[face].index[j]);
            }
        }

        LayerZero->SetNormals(LayerElementNormal);
    } else {
        mesh->GenerateNormals(true, true);
    }

    // Create and fill in the per-face-vertex texture coordinate data source(s).
    // Create UV for Diffuse channel.
    int TexCoordSourceIndex = 0;
    const int32_t TexCoordSourceCount = poly.channels.size();
    for (auto it = poly.channels.begin(); it != poly.channels.end(); ++it) {
        if (it->first == 0) {
            // Create and fill in the vertex color data source.
            FbxLayerElementVertexColor* VertexColor = FbxLayerElementVertexColor::Create(mesh, "");
            VertexColor->SetMappingMode(FbxLayerElement::eByPolygonVertex);
            VertexColor->SetReferenceMode(FbxLayerElement::eIndexToDirect);

            for (int32_t VertIndex = 0; VertIndex < it->second.values.size(); ++VertIndex) {
                const auto& VertColor = it->second.values[VertIndex];
                VertexColor->GetDirectArray().Add(FbxColor(VertColor.x, VertColor.y, VertColor.z, VertColor.reserved));
            }
            for (int32_t face = 0; face < it->second.faces.size(); ++face) {
                for (int32_t j = 0; j < it->second.faces[face].index.size(); ++j) {
                    VertexColor->GetIndexArray().Add(it->second.faces[face].index[j]);
                }
            }
            LayerZero->SetVertexColors(VertexColor);
        } else {
            FbxLayer* Layer = mesh->GetLayer(TexCoordSourceIndex);
            if (Layer == NULL) {
                auto ret = mesh->CreateLayer();
                Layer = mesh->GetLayer(TexCoordSourceIndex);
                if (Layer) TexCoordSourceIndex++;
            }

            std::string name = "UV" + std::to_string(TexCoordSourceIndex);

            FbxLayerElementUV* UVLayer = FbxLayerElementUV::Create(mesh, name.c_str());
            UVLayer->SetMappingMode(FbxLayerElement::eByPolygonVertex);
            UVLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

            // Create the texture coordinate data source.
            for (int32_t TexCoordIndex = 0; TexCoordIndex < it->second.values.size(); ++TexCoordIndex) {
                const auto& TexCoord = it->second.values[TexCoordIndex];
                UVLayer->GetDirectArray().Add(FbxVector2(TexCoord.x, TexCoord.y));
            }
            for (int32_t face = 0; face < it->second.faces.size(); ++face) {
                for (int32_t j = 0; j < it->second.faces[face].index.size(); ++j) {
                    UVLayer->GetIndexArray().Add(it->second.faces[face].index[j]);
                }
            }

            Layer->SetUVs(UVLayer, FbxLayerElement::eTextureDiffuse);
        }
    }

    SetElementMaterial(mesh, poly, mat_node, _mgr, myScene);
    //FbxLayerElementMaterial* MatLayer = FbxLayerElementMaterial::Create(mesh, "");
    //MatLayer->SetMappingMode(FbxLayerElement::eByPolygon);
    //MatLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    //LayerZero->SetMaterials(MatLayer);

    return mesh;
}

EditablePoly ConvertToPolyFrom(const EditableMesh & mesh) {
    EditablePoly ret;

    ret.id = mesh.id;
    ret.parent_id = mesh.parent_id;
    ret.mesh_name = mesh.mesh_name;
    auto assign_func_points = [](const std::vector<EditableMesh::coordinate>& src, std::vector<EditablePoly::coordinate>& dest) {
        dest.clear();
        for (const auto& m : src) {
            dest.emplace_back();
            auto& p = dest.back();
            p.x = m.x;
            p.y = m.y;
            p.z = m.z;
            p.reserved = 1.0;
        }
    };

    auto assign_func_faces = [](const std::vector<EditableMesh::triangle>& src, std::vector<EditablePoly::face>& dest) {
        dest.clear();
        for (const auto& m : src) {
            dest.emplace_back();
            auto& p = dest.back();
            p.index.push_back(m.v1);
            p.index.push_back(m.v2);
            p.index.push_back(m.v3);
            p.smooth_group = m.smooth_group;
            p.submesh_index = m.submesh_index;
        }
    };

    assign_func_points(mesh.points, ret.points);
    assign_func_faces(mesh.triangles, ret.faces);
    assign_func_points(mesh.normal_points, ret.normal_points);
    assign_func_faces(mesh.normal_triangles, ret.normal_faces);
    for (const auto & m : mesh.channels) {
        std::pair<int, EditablePoly::MapChannel> data;
        data.first = m.first;
        assign_func_points(m.second.values, data.second.values);
        assign_func_faces(m.second.triangles, data.second.faces);
        ret.channels.emplace(std::move(data));
    }

    return ret;
}

FBXSDK_NAMESPACE::FbxMesh * ToFbxMesh(const std::shared_ptr<max::SceneGraphNode>& mesh_node, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager *_mgr, FBXSDK_NAMESPACE::FbxScene *_scene) {
    if (mesh_node) {
        if (IsNodeType<EditableMesh>(mesh_node)) {
            return ToFbxMesh(EditableMesh::CreateFrom(mesh_node), mat_node, _mgr, _scene);
        }
        if (IsNodeType<EditablePoly>(mesh_node)) {
            return ToFbxMesh(EditablePoly::CreateFrom(mesh_node), mat_node, _mgr, _scene);
        }
    }
    return nullptr;
}

FBXSDK_NAMESPACE::FbxMesh * ToFbxMesh(const max::EditableMesh & mesh, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager *_mgr, FBXSDK_NAMESPACE::FbxScene *_scene) {
    return ToFbxMesh(ConvertToPolyFrom(mesh), mat_node, _mgr, _scene);
}

bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh *mesh, const max::EditablePoly & poly, const max::StandardMaterial & single, FBXSDK_NAMESPACE::FbxManager *, FBXSDK_NAMESPACE::FbxScene *) {
    // only set layer index 0.
    // remove first
    if (mesh->GetElementMaterial()) {
        mesh->RemoveElementMaterial(mesh->GetElementMaterial());
    }
    // create
    auto elementMaterial = mesh->CreateElementMaterial();
    elementMaterial->SetReferenceMode(FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect);
    elementMaterial->SetMappingMode(FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode::eAllSame);

    FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& materialIndice = elementMaterial->GetIndexArray();
    materialIndice.Add(0);
    //elementMaterial->SetName(single.thumbnail.name.c_str());

    auto node = mesh->GetNode();
    if (node) {
        FBXSDK_NAMESPACE::FbxSurfaceMaterial* mat = FBXSDK_NAMESPACE::FbxSurfaceMaterial::Create(node, single.thumbnail.name.c_str());
        mat->SetUserDataPtr(0);
        node->AddMaterial(mat);
    }

    return true;
}

bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh *mesh, const max::EditablePoly & poly, const max::MultiMaterial & multi, FBXSDK_NAMESPACE::FbxManager *, FBXSDK_NAMESPACE::FbxScene *) {
    // only set layer index 0.
    // remove first
    if (mesh->GetElementMaterial()) {
        mesh->RemoveElementMaterial(mesh->GetElementMaterial());
    }
    // create
    auto elementMaterial = mesh->CreateElementMaterial();
    elementMaterial->SetReferenceMode(FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect);
    elementMaterial->SetMappingMode(FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode::eByPolygon);

    FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& materialIndice = elementMaterial->GetIndexArray();
    std::map<int, int> compressor; // submesh_index --> stored_vector_index
    int material_index = 0;
    bool has_default = false;
    for (const auto& m : poly.faces) {
        if (multi.sub_materials.size()) {
            auto it = multi.sub_materials.find(m.submesh_index);
            if (it == multi.sub_materials.end() || !it->second.enabled) {
                // 没有对应材质。但它依旧是个submesh，统一default。
                // 或被禁用
                materialIndice.Add(-1);
                has_default = true;
            } else {
                // 有对应材质
                auto index_it = compressor.find(m.submesh_index);
                if (index_it == compressor.end()) {
                    materialIndice.Add(material_index);
                    compressor.emplace(m.submesh_index, material_index++);
                } else {
                    materialIndice.Add(index_it->second);
                }
            }
        }
    }
    // 最后一个应为default
    if (has_default) {
        compressor.emplace(-1, material_index);
        for (int i = 0; i < materialIndice.GetCount(); ++i) {
            if (materialIndice.GetAt(i) == -1) {
                materialIndice.SetAt(i, material_index);
            }
        }
    }

    auto node = mesh->GetNode();
    if (node) {
        for (const auto& m : compressor) {
            auto it = multi.sub_materials.find(m.first);
            if (it != multi.sub_materials.end()) {
                FBXSDK_NAMESPACE::FbxSurfaceMaterial* mat = FBXSDK_NAMESPACE::FbxSurfaceMaterial::Create(node, it->second.name.c_str());
                int64_t val;
                int32_t* vptr = (int32_t*)&val; vptr[0] = m.first; vptr[1] = m.second;
                mat->SetUserDataPtr((void*)val); // submesh index, vec index
                node->AddMaterial(mat);
            }
        }
    }

    //elementMaterial->SetName(single.thumbnail.name.c_str());
    return true;
}

bool SetElementMaterial(FBXSDK_NAMESPACE::FbxMesh *mesh, const max::EditablePoly & poly, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager *_mgr, FBXSDK_NAMESPACE::FbxScene *_scene) {
    bool ret = false;
    if (mesh) {
        // add if has mat_node
        if (mat_node) {
            if (IsNodeType<StandardMaterial>(mat_node)) {
                ret = SetElementMaterial(mesh, poly, StandardMaterial::CreateFrom(mat_node), _mgr, _scene);
            }
            if (IsNodeType<MultiMaterial>(mat_node)) {
                ret = SetElementMaterial(mesh, poly, MultiMaterial::CreateFrom(mat_node), _mgr, _scene);
            }
        }
    }
    return ret;
}