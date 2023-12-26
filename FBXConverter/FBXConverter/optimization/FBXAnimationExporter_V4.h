#pragma once
#include "CommonStruct.h"

class JSONFileWriter;

class FBXAnimationExporter_V4
{
public:
	FBXAnimationExporter_V4();
	~FBXAnimationExporter_V4();

	bool init(FBXSDK_NAMESPACE::FbxManager* manager, FBXSDK_NAMESPACE::FbxScene* scene);
	void finl();

	inline std::vector<KeyframeAnimation>& getAnimations()
	{
		return mAnimations;
	}

private:
	void getAnimationSingleClip(
		FBXSDK_NAMESPACE::FbxAnimStack* animationStack,
		FBXSDK_NAMESPACE::FbxNode* root,
		std::vector<animationClipData>* container);

	std::string getNodePath(FBXSDK_NAMESPACE::FbxNode* node);

	void getNodeAnimation_r(
		FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
		FBXSDK_NAMESPACE::FbxNode* node,
		std::vector<animationClipData>* container);

	std::vector<float>** getTRSVAnimationData(
		FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
		FBXSDK_NAMESPACE::FbxNode* node);

	std::vector<morphTargetAnimation*>* getMophAnimationData(
		FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
		FBXSDK_NAMESPACE::FbxMesh* mesh);

	void getKeyframes(
		FBXSDK_NAMESPACE::FbxAnimCurve* animationCurve, 
		std::vector<float>& keyframes);

	FBXSDK_NAMESPACE::FbxTime::EMode mTimeMode;
	std::vector<KeyframeAnimation>	mAnimations;

	unsigned int keyFrameCount;
};

