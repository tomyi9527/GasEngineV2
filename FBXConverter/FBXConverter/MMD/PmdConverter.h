#pragma once
#include "PmdReader.h"
#include "CommonStruct.h"

class JSONFileWriter;
class SceneNode;
class IKChain;

class PmdConverter
{
public:
	PmdConverter();
	~PmdConverter();

	bool convert(
		const std::string& workingDirectory,
		const std::string& filePath,
		unsigned int optimizationFlag,
		std::string& background);

	void buildSceneStructure(pmd::PmdModel* pmd);

	std::vector<BONE_>* createSkeleton(std::vector<pmd::PmdBone>& bones);

	void getMorph(
		std::vector<pmd::PmdMorph>& morphs,
		float* originalPositions,
		unsigned int vertexCount,
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMax,
		std::vector<MORPH_DATA_V2>* morphTargetsData);

	void convertSceneFile(
		const std::string& workingDirectory,
		void* pmd,
		std::vector<std::string>& convertedFiles,
		const std::string& background,
		int version);

	SceneNode* sceneRoot;
	std::vector<SceneNode*> nodes;
	int mVersion;
};