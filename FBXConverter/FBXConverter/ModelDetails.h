#pragma once
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include "CommonStruct.h"
#include "JSONFileWriter.h"

class ModelDetails
{
public:
	ModelDetails();
	~ModelDetails();
	void save(const std::string& directoryPath, const std::string& modelFileName);

	void setMeshDetails(std::vector<MESH_DETAIL>* meshDetails)
	{
		mMeshDetails = meshDetails;
	}

	void setAnimationDetails(std::vector<ANIMATION_DETAIL>* animationDetails)
	{
		mAnimationDetails = animationDetails;
	}

	void setTextures(std::vector<std::string>* textures)
	{
		mTextures = textures;
	}

protected:
	bool writeOutput(const std::string& filePath, bool isShowVertexFormat);

private:
	int mTotalTriangleCount;
	int mTotalPolygonCount;
	int mTotalVertexCount;
	int mTotalBlendShapeCount;
	int mTotalKeyFrameCount;
	float mFPS;
	float mAnimationDuration;

	std::vector<std::string>* mTextures;
	std::set<int> mTotalVertexFormats;
	std::map<int, std::string> mTotalBones;
	std::map<int, int> mBoneHierarchy;

	std::vector<MESH_DETAIL>* mMeshDetails;
	std::vector<ANIMATION_DETAIL>* mAnimationDetails;
};

