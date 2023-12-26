#pragma once

class JSONFileWriter;

class FBXAnimationExporter_V2
{
public:
	FBXAnimationExporter_V2();
	~FBXAnimationExporter_V2();

	bool init(const string& path, FBXSDK_NAMESPACE::FbxManager* manager, FBXSDK_NAMESPACE::FbxScene* scene, std::map<std::string, unsigned int>& convertedFiles);
	void finl();

private:
	bool isObjectAnimated(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, \
		int& translationKeyCount, int& rotationKeyCount, int& scalingKeyCount, int& morphWeightKeyCount);
	void gatherAnimatedNode_r(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxNode* node);
	void exportAnimationToJSON(FBXSDK_NAMESPACE::FbxScene* pScene);
	void processAnimation(FBXSDK_NAMESPACE::FbxAnimStack* pAnimStack, FBXSDK_NAMESPACE::FbxNode* node);
	void writeAnimationData(FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, FBXSDK_NAMESPACE::FbxObject* object, int jsonLevel, bool lastChild);
	void gatherAllTimeKeys(FBXSDK_NAMESPACE::FbxAnimCurve* lAnimCurve, std::map<FbxLongLong, int>& timeKeys);
	void iterateCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, int jsonLevel);
	FBXSDK_NAMESPACE::FbxAnimCurve* iterateMorphWeightCurves(FBXSDK_NAMESPACE::FbxMesh* mesh, int channelIndex, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, int>& timeKeys);
	void iteratePositionCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timeKeys);
	void iterateRotationCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timeKeys);
	void iterateScalingCurves(FBXSDK_NAMESPACE::FbxNode* node, FBXSDK_NAMESPACE::FbxAnimLayer* pAnimLayer, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timeKeys);
	void exportPositionKeys(FBXSDK_NAMESPACE::FbxNode* node, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timePositionKeys, int jsonLevel);
	void exportRotationKeys(FBXSDK_NAMESPACE::FbxNode* node, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timeRotationKeys, int jsonLevel);
	void exportScalingKeys(FBXSDK_NAMESPACE::FbxNode* node, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timeScalingKeys, int jsonLevel);
	void writeKeysInJSON(FBXSDK_NAMESPACE::FbxNode* node, std::map<FBXSDK_NAMESPACE::FbxLongLong, int>& timePositionKeys, int type, int jsonLevel);

	void exportMorphWeightKeys(const char* channelName, int channelIndex, FbxAnimCurve* aniCurve, std::map<FbxLongLong, int>& timeMorphWeightKeys, int jsonLevel, bool isLastChannel);
	void writeMorphWeightKeysInJSON(FbxAnimCurve* aniCurve, std::map<FbxLongLong, int>& timeMorphWeightKeys, int jsonLevel);

	std::vector<FbxObject*>			mAnimationNode;

	JSONFileWriter*					mSceneStructureJSONWriter;
};

