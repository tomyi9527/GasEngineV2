#include "stdafx.h"
#include "MAXSceneStructureExporter_V1.h"
#include "Common/Common.h"
#include "MAX/max_reader.h"
#include "MaxMeshLoader.h"
#include "AssistantFunctions.h"
#include "JSONFileWriter.h"
#include "Material.h"
#include "NeonateVertexCompression_V4.h"
#include "TextureMap.h"
#include "meshLoader.h"
#include "JsonToBin_V4.h"

using namespace max;

MAXSceneStructureExporter_V1::MAXSceneStructureExporter_V1()
    : mGeneratedMaterialCount(0), mFbxMgr(nullptr), mFbxScene(nullptr) {

}

MAXSceneStructureExporter_V1::~MAXSceneStructureExporter_V1() {
}

bool MAXSceneStructureExporter_V1::init(const std::shared_ptr<SceneGraphNode>& node) {
    mMaxScene = node;
    mMaxID = mMaxScene->data_ptr->id + 1; // scene 拥有最大 id

    auto scene = Scene::CreateFrom(mMaxScene);

    //Textures and Materials
    mUsedMaterialsInTheScene.clear();
    std::shared_ptr<SceneGraphNode> rootNode = scene.GetRoot();

    mGeneratedMaterialCount = 0;
    gatherAllInformation_r(rootNode, mEffectiveBones, nullptr);

    for (int i = 0; i < (int)mUsedMaterialsInTheScene.size(); ++i) {
        Material* material = new Material();
        material->parseMaxMaterial(mUsedMaterialsInTheScene[i].get(), mTextureMaps, mTextureFiles);
        mMaterials.push_back(material);
    }

    if (mNodes.size()) {
        // root node 本身不会有 rotation，因此全局旋转可以放这里
        mNodes.front()->rotation[0] = -90.0; 
        mNodes.front()->rotation[1] = 0.0;
        mNodes.front()->rotation[2] = 0.0;
    }

    for (size_t s = 0; s < mNodes.size(); ++s) {
        ::Node* node = mNodes[s];
        if (node->parent >= 0) {
            mNodes[node->parent]->children.push_back((int)s);
        }
    }

    return true;
}

void MAXSceneStructureExporter_V1::finl() {
    for (int i = 0; i < (int)mNodes.size(); ++i) {
        delete mNodes[i];
    }
    mNodes.clear();

    for (int i = 0; i < (int)mTextureMaps.size(); ++i) {
        delete mTextureMaps[i];
    }
    mTextureMaps.clear();

    for (int i = 0; i < (int)mMaterials.size(); ++i) {
        delete mMaterials[i];
    }
    mMaterials.clear();

    mEffectiveBones.clear();
}

template<typename AnyScalar>
inline FBXSDK_NAMESPACE::FbxVector4 ConvertArray(const std::array<AnyScalar, 3>& v) {
    return FBXSDK_NAMESPACE::FbxVector4((double)v[0], (double)v[1], (double)v[2]);
}

template<typename AnyScalar>
inline FBXSDK_NAMESPACE::FbxVector4 ConvertArray(const std::array<AnyScalar, 4>& v) {
    return FBXSDK_NAMESPACE::FbxVector4((double)v[0], (double)v[1], (double)v[2], (double)v[3]);
}

void MAXSceneStructureExporter_V1::gatherAllInformation_r(
    const std::shared_ptr<SceneGraphNode>& node_base,
    std::vector<unsigned int>& effectiveBones,
    ::Node* parentNode
) {
    if (!max::IsNodeType<max::Node>(node_base) && !max::IsNodeType<max::RootNode>(node_base)) {
        return;
    }
    max::Node node = max::Node::CreateFrom(node_base);
    bool vis = !node.IsHide();
    if (node.layer_node && vis) {
        vis = !max::BaseLayer::CreateFrom(node.layer_node).IsHide();
    }

    // node part
    ::Node* myNode = new ::Node();
    mNodes.push_back(myNode);
    myNode->_index_ = (int)mNodes.size() - 1;
    if (parentNode != NULL) {
        myNode->parent = parentNode->_index_;
    } else {
        myNode->parent = -1;
    }

    myNode->uniuqeID = node.id; //<

    std::string guidString = newUUID();
    myNode->guid = guidString; //<

    string nodeName = standardizeFileName(node.name);
    myNode->name = nodeName;

    myNode->meshInfo = NULL;
    myNode->skeletonName = "eNone";

    // component: SRT
    myNode->is_max = true;
    if (node.sRT_node) {
        auto sRT_converted = PositionRotationScale::CreateFrom(node.sRT_node);
        myNode->translation = ConvertArray(sRT_converted.position.GetValue());
        myNode->rotation = ConvertArray(sRT_converted.rotation.GetDegreeValue());
        myNode->scaling = ConvertArray(sRT_converted.scale.GetFactor());
        myNode->scaling_axis = ConvertArray(sRT_converted.scale.GetAxis());
    } else {
        myNode->translation = ConvertArray(std::array<float, 3>{0.0f, 0.0f, 0.0f});
        myNode->rotation = ConvertArray(std::array<float, 3>{0.0f, 0.0f, 0.0f});
        myNode->scaling = ConvertArray(std::array<float, 3>{1.0f, 1.0f, 1.0f});
        myNode->scaling_axis = ConvertArray(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    }
    myNode->offset_rotation = ConvertArray(node.pivot_rotation);
    myNode->offset_translation = ConvertArray(node.pivot_position);
    myNode->offset_scaling = ConvertArray(node.pivot_scale);
    myNode->offset_scaling_axis = ConvertArray(node.pivot_scale_quat_axis);

    // component: mesh and material
    if (vis) {
        int meshUniqueID = -1;
        if (node.mesh_node) {
            FBXSDK_NAMESPACE::FbxMesh* mesh = nullptr;
            if (IsNodeType<EditableMesh>(node.mesh_node)) {
                auto max_mesh = EditableMesh::CreateFrom(node.mesh_node);
                meshUniqueID = max_mesh.id;
                mesh = ToFbxMesh(max_mesh, node.material_node, mFbxMgr, mFbxScene);
            }
            if (IsNodeType<EditablePoly>(node.mesh_node)) {
                auto max_poly = EditablePoly::CreateFrom(node.mesh_node);
                meshUniqueID = max_poly.id;
                mesh = ToFbxMesh(max_poly, node.material_node, mFbxMgr, mFbxScene);
            }

            if (meshUniqueID != -1) {
                // has parsed mesh
                int polygonCount = mesh->GetPolygonCount();
                if (polygonCount > 0) {
                    NeonateVertexCompression_V4::getEffectiveBones(mesh, effectiveBones);
                }

                std::map<int, MultiMaterial::SubMaterialItem> material_items;
                // add default record
                MultiMaterial::SubMaterialItem item;
                item.id = -1;
                item.component_key = 0;
                item.enabled = true;
                item.target_material_node = nullptr;
                material_items.emplace(-1, std::move(item));

                // add from material_node
                if (node.material_node) {
                    // has material
                    if (IsNodeType<StandardMaterial>(node.material_node)) {
                        MultiMaterial::SubMaterialItem item;
                        item.id = 0; // submesh id is always 0
                        item.component_key = 0;
                        item.enabled = true;
                        item.target_material_node = node.material_node;
                        material_items.emplace(0, std::move(item));
                    } else if (IsNodeType<MultiMaterial>(node.material_node)) {
                        auto multi = MultiMaterial::CreateFrom(node.material_node);
                        for (const auto& m : multi.sub_materials) {
                            material_items.emplace(m.second.id, m.second);
                        }
                    } else {
                        // unsupported material
                        FBXSDK_printf("Error: Detected a mesh with unsupported material! Will generate a default one for it!\n");
                    }
                } else {
                    // no material
                    FBXSDK_printf("Error: Detected a mesh without material! Generated a default one for it!\n");
                    char buffer[__TEMP_BUFFER_FLOAT__];
                    sprintf(buffer, "Unknown_%d", mGeneratedMaterialCount);
                    ++mGeneratedMaterialCount;
                }

                // gather mesh subindex 
                auto node = mesh->GetNode();
                std::vector<std::unique_ptr<StandardMaterial>> materials;
                const int materialCount = node->GetMaterialCount();
                for (int i = 0; i < materialCount; ++i) {
                    FBXSDK_NAMESPACE::FbxSurfaceMaterial* material = node->GetMaterial(i);
                    int64_t val = (int64_t)material->GetUserDataPtr();
                    int32_t val_pair[2];
                    val_pair[0] = ((int32_t*)&val)[0]; // submesh index
                    val_pair[1] = ((int32_t*)&val)[1]; // stored index

                    auto it = material_items.find(val_pair[0]);
                    if (it == material_items.end()) {
                        FBXSDK_printf("Error: This should not happen.\n");
                    }
                    if (materials.size() <= val_pair[1]) {
                        materials.resize(val_pair[1] + 1);
                    }
                    materials[val_pair[1]] = std::make_unique<StandardMaterial>(
                        StandardMaterial::CreateFrom(it->second.target_material_node)
                        );
                    if (!it->second.target_material_node) {
                        materials[val_pair[1]]->id = mMaxID; // for default
                    }
                }

                // use default material
                if (materials.empty()) {
                    materials.push_back(std::make_unique<StandardMaterial>(
                        StandardMaterial::CreateFrom(nullptr)
                        ));
                    materials.back()->id = mMaxID;
                }

                for (auto& m : materials) {
                    uint32_t material_id = m->id;
                    bool flag = false;
                    uint32_t index = 0;
                    for (int j = 0; j < (int)mUsedMaterialsInTheScene.size(); ++j) {
                        auto id = mUsedMaterialsInTheScene[j]->id;
                        if (material_id == id) {
                            flag = true; // found
                            index = j;
                            break;
                        }
                    }

                    if (!flag) {
                        // create new.
                        mUsedMaterialsInTheScene.push_back(std::move(m));
                        index = mUsedMaterialsInTheScene.size() - 1;
                    }

                    // TODO(beanpliu): 缺陷：目前 FbxMesh / FbxNode 的 id 与 max_parser 给出的 id 是不同的，
                    // 因此如果需要导出正确 id 的话，需要特殊处理。建议后续改为将 id, parent_id 存在 MeshInfo 内。
                    // 目前导出是使用的文件名是 mesh 中的 id，因此此处先将 id 替换。
                    meshUniqueID = mesh->GetUniqueID();

                    std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
                    if (iter != mMeshDepot.end()) {
                        iter->second.mesh = mesh;
                        iter->second.meshName = mesh->GetNode()->GetName();
                        iter->second.materials.push_back(index);
                    } else {
                        MeshInfo info;
                        info.uniqueID = meshUniqueID;
                        info.mesh = mesh;
                        info.meshName = mesh->GetNode()->GetName();
                        info.materials.push_back(index);
                        mMeshDepot[meshUniqueID] = std::move(info);
                    }

                    myNode->materials.push_back(index);
                }
            }

            std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
            if (iter != mMeshDepot.end()) {
                myNode->meshInfo = &(iter->second);
            } else {
                FBXSDK_printf("Error: found a dummy mesh.\n");
            }
        }
    }

    //if (skeletonType == FbxSkeleton::eRoot) {
    //    myNode->skeletonName = "eRoot";
    //} else if (skeletonType == FbxSkeleton::eLimb) {
    //    myNode->skeletonName = "eLimb";
    //} else if (skeletonType == FbxSkeleton::eLimbNode) {
    //    myNode->skeletonName = "eLimbNode";
    //} else if (skeletonType == FbxSkeleton::eEffector) {
    //    myNode->skeletonName = "eEffector";
    //}

    // traverse recursively
    for (const auto& m : node_base->children) {
        gatherAllInformation_r(m, effectiveBones, myNode);
    }
}