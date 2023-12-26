#pragma once
#include "max_structure.h"
#include "fbxsdk.h"
#include "CommonStruct.h"

// TODO(beanpliu): write smooth info
max::EditablePoly ConvertToPolyFrom(const max::EditableMesh& mesh);
FBXSDK_NAMESPACE::FbxMesh* ToFbxMesh(const std::shared_ptr<max::SceneGraphNode>& mesh_node, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);
FBXSDK_NAMESPACE::FbxMesh* ToFbxMesh(const max::EditableMesh& mesh, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);
FBXSDK_NAMESPACE::FbxMesh* ToFbxMesh(const max::EditablePoly& poly, const std::shared_ptr<max::SceneGraphNode>& mat_node, FBXSDK_NAMESPACE::FbxManager*, FBXSDK_NAMESPACE::FbxScene*);
