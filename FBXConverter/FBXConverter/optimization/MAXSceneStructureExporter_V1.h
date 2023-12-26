#pragma once
#include "CommonStruct.h"
#include "fbxsdk.h"
#include <memory>
#include <map>
#include <vector>
#include <memory>

class Material;
class TextureMap;
class Node;
namespace max {
class SceneGraphNode;
class StandardMaterial;
}

class MAXSceneStructureExporter_V1 {
public:
    MAXSceneStructureExporter_V1();
    ~MAXSceneStructureExporter_V1();

    bool init(const std::shared_ptr<max::SceneGraphNode>& node);
    void finl();

    inline bool initFBXHelper(FBXSDK_NAMESPACE::FbxManager* manager, FBXSDK_NAMESPACE::FbxScene* scene) {
        mFbxMgr = manager;
        mFbxScene = scene;
        return mFbxMgr && mFbxScene;
    }

    inline std::vector<::Node*>& getNodes() {
        return mNodes;
    }

    inline std::vector<Material*>& getMaterials() {
        return mMaterials;
    }

    inline std::map<int, MeshInfo>& getMeshes() {
        return mMeshDepot;
    }

    inline std::vector<TextureMap*>& getTextureMaps() {
        return mTextureMaps;
    }

    inline std::vector<std::string>& getTextures() {
        return mTextureFiles;
    }

    inline std::vector<unsigned int>& getEffectiveBones() {
        return mEffectiveBones;
    }

private:
    void gatherAllInformation_r(const std::shared_ptr<max::SceneGraphNode>& node, std::vector<unsigned int>& effectiveBones, ::Node* parentNode);

    std::vector<std::unique_ptr<max::StandardMaterial>> mUsedMaterialsInTheScene;

    std::shared_ptr<max::SceneGraphNode> mMaxScene;
    FBXSDK_NAMESPACE::FbxManager* mFbxMgr;
    FBXSDK_NAMESPACE::FbxScene* mFbxScene;

    std::vector<::Node*>		mNodes;
    std::vector<Material*>		mMaterials;
    std::vector<TextureMap*>	mTextureMaps;
    std::map<int, MeshInfo>		mMeshDepot;
    std::vector<std::string>	mTextureFiles;

    std::vector<unsigned int>	mEffectiveBones;

    int							mGeneratedMaterialCount;
    int                         mMaxID;
};