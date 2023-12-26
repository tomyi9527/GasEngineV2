#pragma once
#include "CommonStruct.h"

class Material;
class TextureMap;
class Node;

class FBXSceneStructureExporter_V4
{
public:
	FBXSceneStructureExporter_V4();
	~FBXSceneStructureExporter_V4();

	bool init(FbxManager* manager, FbxScene* scene);
	void finl();

	inline std::vector<Node*>& getNodes()
	{
		return mNodes;
	}

	inline std::vector<Material*>& getMaterials()
	{
		return mMaterials;
	}

	inline std::map<int, MeshInfo>& getMeshes()
	{
		return mMeshDepot;
	}

	inline std::vector<TextureMap*>& getTextureMaps()
	{
		return mTextureMaps;
	}

	inline std::vector<std::string>& getTextures()
	{
		return mTextureFiles;
	}

	inline std::vector<unsigned int>& getEffectiveBones()
	{
		return mEffectiveBones;
	}

private:
	void gatherAllUsedMaterils_r(FBXSDK_NAMESPACE::FbxNode* node, std::vector<unsigned int>& effectiveBones);
	void collectHierarchicalInfo_r(FBXSDK_NAMESPACE::FbxNode* node, Node* parentNode);
	std::vector<FBXSDK_NAMESPACE::FbxSurfaceMaterial*> mUsedMaterialsInTheScene;

	FbxManager*					mFbxManager;
	FbxScene*					mFbxScene;

	std::vector<Node*>			mNodes;
	std::vector<Material*>		mMaterials;
	std::vector<TextureMap*>	mTextureMaps;
	std::map<int, MeshInfo>		mMeshDepot;
	std::vector<std::string>	mTextureFiles;

	std::vector<unsigned int>	mEffectiveBones;

	int							mGeneratedMaterialCount;
};