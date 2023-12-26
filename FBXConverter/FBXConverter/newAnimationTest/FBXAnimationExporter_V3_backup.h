#pragma once

class JSONFileWriter;

class FBXAnimationExporter
{
public:
	FBXAnimationExporter();
	~FBXAnimationExporter();

	bool init(const string& path, FBXSDK_NAMESPACE::FbxManager* manager, FBXSDK_NAMESPACE::FbxScene* scene, std::map<std::string, unsigned int>& convertedFiles);
	void finl();

private:
	bool isObjectAnimated(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, \
		int& translationKeyCount, int& rotationKeyCount, int& scalingKeyCount, int& morphWeightKeyCount);
	void gatherAnimatedNode_r(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxNode* node);
	void exportAnimationToJSON(FBXSDK_NAMESPACE::FbxScene* pScene);
	void processAnimation(FBXSDK_NAMESPACE::FbxAnimStack* pAnimStack, FBXSDK_NAMESPACE::FbxNode* node);
	void writeAnimationData(FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, FBXSDK_NAMESPACE::FbxObject* object, int jsonLevel, bool lastChild);
	void gatherAllTimeKeys(FBXSDK_NAMESPACE::FbxAnimCurve* lAnimCurve, std::map<FbxLongLong, float>& timeKeys);
	void iterateCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, int jsonLevel);
	FBXSDK_NAMESPACE::FbxAnimCurve* iterateMorphWeightCurves(FBXSDK_NAMESPACE::FbxMesh* mesh, int channelIndex, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, float>& timeKeys);

	void iteratePositionCurves(FbxNode* node, FbxAnimLayer* pAnimLayer, 
		std::map<FbxLongLong, float>& positionX, std::map<FbxLongLong, float>& positionY, std::map<FbxLongLong, float>& positionZ);

	void iterateRotationCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer,
		std::map<FbxLongLong, float>& rotationX, std::map<FbxLongLong, float>& rotationY, std::map<FbxLongLong, float>& rotationZ);

	void iterateScalingCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer,
		std::map<FbxLongLong, float>& scalingX, std::map<FbxLongLong, float>& scalingY, std::map<FbxLongLong, float>& scalingZ);

	void FBXAnimationExporter::exportKeys(FbxNode* node, std::string elementName, std::map<FbxLongLong, float>& keyframes, int jsonLevel, bool isLastElement);

	void exportPositionKeys(FBXSDK_NAMESPACE::FbxNode* node, 
		std::map<FbxLongLong, float>& positionX, std::map<FbxLongLong, float>& positionY, std::map<FbxLongLong, float>& positionZ, int jsonLevel);

	void exportRotationKeys(FBXSDK_NAMESPACE::FbxNode* node,
		std::map<FbxLongLong, float>& rotationX, std::map<FbxLongLong, float>& rotationY, std::map<FbxLongLong, float>& rotationZ, int jsonLevel);

	void exportScalingKeys(FBXSDK_NAMESPACE::FbxNode* node,
		std::map<FbxLongLong, float>& scalingX, std::map<FbxLongLong, float>& scalingY, std::map<FbxLongLong, float>& scalingZ, int jsonLevel);

	void writeKeysInJSON(FBXSDK_NAMESPACE::FbxNode* node, std::map<FBXSDK_NAMESPACE::FbxLongLong, float>& keyframes, int jsonLevel);

	void exportMorphWeightKeys(const char* channelName, int channelIndex, FbxAnimCurve* aniCurve, std::map<FbxLongLong, float>& timeMorphWeightKeys, int jsonLevel, bool isLastChannel);
	void writeMorphWeightKeysInJSON(FbxAnimCurve* aniCurve, std::map<FbxLongLong, float>& timeMorphWeightKeys, int jsonLevel);

	std::vector<FbxObject*>			mAnimationNode;

	JSONFileWriter*					mSceneStructureJSONWriter;
};

